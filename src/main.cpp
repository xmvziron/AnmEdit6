#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "AnmManager.hpp"
#include "FileSystem.hpp"
#include "graphics/FixedFunctionGL.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl2.h"

/* yes, we use 2 SDLs, i know */
#include <SDL2/SDL.h>
#include <SDL3/SDL_dialog.h>

#include <vector>

using namespace th06;

Supervisor th06::g_Supervisor;

#define MAIN_WINDOW_TITLE  "AnmEdit6"
#define MAIN_WINDOW_WIDTH  960
#define MAIN_WINDOW_HEIGHT 720

static struct
{
    const char *name;
    bool isEsContext;
    void (*setContextFlags)();
    GfxInterface *(*init)();
} g_FixedFunctionBackend = {"Fixed function GL(ES)", false, FixedFunctionGL::SetContextFlags, FixedFunctionGL::Init};

static SDL_DialogFileFilter g_OpenFileFilters[] =
{
    { "ANM files", "anm" },
    { "All files", "*" }
};

#define FOR_EACH_INSTRUCTION(instruction, sprite) \
    for (AnmRawInstr *instruction = sprite.beginingOfScript; \
    instruction && instruction->opcode != AnmOpcode_Exit;               \
    instruction = (AnmRawInstr *)(((u8 *)instruction->args) + instruction->argsCount))

struct MainWindow
{
    bool running;
    SDL_Window *window;
    SDL_GLContext gameContext;
    SDL_GLContext guiContext;
    AnmManager *anmManager;
    AnmVm anmVm;
    ImVec2 spriteViewPosition;
    std::vector<int> interruptNumbers;
    SDL_mutex *reloadMutex;
    bool scriptIsUsingAbsolutePositions;
    bool needsReload;
    char anmFileName[64];

    bool Init(void)
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            return false;
        }

        this->window = SDL_CreateWindow(MAIN_WINDOW_TITLE,
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        MAIN_WINDOW_WIDTH,
                                        MAIN_WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

        if (!this->window)
        {
            return false;
        }

        /* we use two different GL contexts because there appaears
         * to be some GL state spillage from ImGui.
         */
        this->gameContext = SDL_GL_CreateContext(window);
        if (!this->gameContext)
        {
            this->Error("Could not create OpenGL context for ANM renderer!");
            return false;
        }

        this->guiContext = SDL_GL_CreateContext(window);
        if (!this->guiContext)
        {
            this->Error("Could not create OpenGL context for GUI!");
            return false;
        }

        SDL_GL_MakeCurrent(window, this->gameContext);

        g_glFuncTable.ResolveFunctions(false);

        g_glFuncTable.glEnable(GL_BLEND);
        g_glFuncTable.glEnable(GL_ALPHA_TEST);
        g_glFuncTable.glEnable(GL_SCISSOR_TEST);

        this->anmManager = new AnmManager;
        anmManager->gfxBackend = g_FixedFunctionBackend.init();

        memset(&this->anmVm, 0, sizeof(AnmVm));

        g_Supervisor.framerateMultiplier = 1.0f;

        SDL_GL_MakeCurrent(window, this->guiContext);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplSDL2_InitForOpenGL(window, guiContext);
        ImGui_ImplOpenGL2_Init();

        this->reloadMutex = SDL_CreateMutex();

        SDL_ShowWindow(this->window);
        this->running = true;

        return true;
    }

    void MainLoop(void)
    {
        while (running)
        {
            SDL_Event event;

            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    running = false;
                }

                ImGui_ImplSDL2_ProcessEvent(&event);
            }
            
            /* Some sprites never use absolute positions,
             * and will display in the top left of the screen.
             * So put them in the center of the screen.
             */
            if (!this->scriptIsUsingAbsolutePositions)
            {
                this->anmVm.pos.x = GAME_WINDOW_WIDTH / 2.0f;
                this->anmVm.pos.y = GAME_WINDOW_HEIGHT / 2.0f;
                this->anmVm.pos.z = 0.0f;
            }

            if (needsReload)
            {
                SDL_LockMutex(this->reloadMutex);
                LoadAnmFile();
                needsReload = false;
                SDL_UnlockMutex(this->reloadMutex);
            }

            this->anmManager->ExecuteScript(&this->anmVm);

            RenderGui();

            RenderSprite();

            SDL_GL_SwapWindow(window);
        }
    }

    void LoadAnmFile(void)
    {
        memset(&this->anmVm, 0, sizeof(AnmVm));
        SDL_GL_MakeCurrent(this->window, this->gameContext);

        this->anmManager->LoadAnm(0, this->anmFileName, 0);

        OnScriptChange(0);
    }

    void OnScriptChange(int scriptNumber)
    {
        this->anmManager->SetAndExecuteScriptIdx(&this->anmVm, scriptNumber);

        this->GetSpriteInterrupts();
        this->scriptIsUsingAbsolutePositions = this->ScriptIsUsingAbsolutePositions();
    }

    void GetSpriteInterrupts(void)
    {
        this->interruptNumbers.clear();

        if (this->anmVm.beginingOfScript == NULL)
        {
            return;
        }

        FOR_EACH_INSTRUCTION(instruction, this->anmVm)
        {
            if (instruction->opcode == AnmOpcode_InterruptLabel)
            {
                this->interruptNumbers.push_back(instruction->args[0]);
            }
        }
    }

    bool ScriptIsUsingAbsolutePositions(void)
    {
        bool result = false;
        bool usePosOffset = false;

        FOR_EACH_INSTRUCTION(instruction, this->anmVm)
        {
            if (instruction->opcode == AnmOpcode_UsePosOffset)
            {
                usePosOffset = instruction->args[0];
            }
            if (!usePosOffset
                && (instruction->opcode >= AnmOpcode_SetPosition
                    && instruction->opcode <= AnmOpcode_PosTimeAccel))
            {
                result = true;
                break;
            }
        }

        return result;
    }

    void RenderGui(void)
    {
        SDL_GL_MakeCurrent(window, this->guiContext);

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        /* TODO: hardcoded border size*/
        ImGui::SetNextWindowSize(ImVec2(GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT + 20));
        ImGui::Begin("Sprite view", NULL, ImGuiWindowFlags_NoResize);
            spriteViewPosition = ImGui::GetWindowPos();
        ImGui::End();

        ImGui::Begin("Sprite debug", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            if (ImGui::BeginTable("Sprite info", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableNextColumn();
                ImGui::Text("Position");
                ImGui::TableNextColumn();
                ImGui::Text("%f %f %f", this->anmVm.pos.x, this->anmVm.pos.y, this->anmVm.pos.z);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("Position offset");
                ImGui::TableNextColumn();
                ImGui::Text("%f %f %f", this->anmVm.posOffset.x, this->anmVm.posOffset.y, this->anmVm.posOffset.z);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("Rotation");
                ImGui::TableNextColumn();
                ImGui::Text("%f %f %f", this->anmVm.rotation.x, this->anmVm.rotation.y, this->anmVm.rotation.z);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("Scale");
                ImGui::TableNextColumn();
                ImGui::Text("%f %f", this->anmVm.scaleX, this->anmVm.scaleY);

                ImGui::EndTable();
            }
        ImGui::End();

        ImGui::Begin("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::InputText("ANM file", anmFileName, sizeof(anmFileName));

            if (ImGui::Button("Open ANM file"))
            {
                SDL_ShowOpenFileDialog(MainWindow::DialogCallback,
                                       this,
                                       this->window,
                                       g_OpenFileFilters,
                                       sizeof(g_OpenFileFilters) / sizeof(SDL_DialogFileFilter),
                                       NULL,
                                       SDL_FALSE);
            }

            if(ImGui::BeginListBox("Scripts"))
            {
                AnmRawEntry *rawEntry = this->anmManager->anmFiles[0];

                if (rawEntry != NULL && rawEntry->numScripts > 0)
                {
                    for (int i = 0; i < rawEntry->numScripts; i++)
                    {
                        char buf[12] = {};

                        snprintf(buf, sizeof(buf), "Script %d", i);

                        if (ImGui::Button(buf))
                        {
                            OnScriptChange(i);
                        }
                    }
                }
                else
                {
                    ImGui::Text("No scripts");
                }

                ImGui::EndListBox();
            }

            if(ImGui::BeginListBox("Interrupts"))
            {
                if (this->interruptNumbers.size() > 0)
                {
                    if (ImGui::Button("Start of script"))
                    {
                        this->anmVm.currentInstruction = this->anmVm.beginingOfScript;
                    }

                    for (int &interruptNumber : this->interruptNumbers)
                    {
                        char buf[32] = {};

                        snprintf(buf, sizeof(buf), "Interrupt %d", interruptNumber);

                        if (ImGui::Button(buf))
                        {
                            this->anmVm.pendingInterrupt = interruptNumber;
                        }
                    }
                }
                else
                {
                    ImGui::Text("No interrupts");
                }

                ImGui::EndListBox();
            }

        ImGui::End();

        ImGui::Render();

        g_glFuncTable.glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        g_glFuncTable.glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    }

    static void DialogCallback(void *data, const char * const *fileList, int filter)
    {
        if (fileList == NULL || fileList[0] == NULL)
        {
            return;
        }

        const char *anmPath = fileList[0];
        MainWindow *_This = (MainWindow *) data;
        strncpy(_This->anmFileName, anmPath, sizeof(_This->anmFileName));

        /* this function is called from NOT the main thread */
        SDL_LockMutex(_This->reloadMutex);
        _This->needsReload = true;
        SDL_UnlockMutex(_This->reloadMutex);

    }

    void RenderSprite(void)
    {
        SDL_GL_MakeCurrent(window, this->gameContext);

        int viewportX = spriteViewPosition.x;
        int viewportY = (MAIN_WINDOW_HEIGHT - spriteViewPosition.y) - GAME_WINDOW_HEIGHT - 20;
        int viewportWidth = GAME_WINDOW_WIDTH;
        int viewportHeight = GAME_WINDOW_HEIGHT;

        g_glFuncTable.glScissor(viewportX, viewportY, viewportWidth, viewportHeight);
        g_glFuncTable.glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
        g_glFuncTable.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        g_glFuncTable.glClear(GL_COLOR_BUFFER_BIT);

        /* crashes if sprite is not valid */
        if (anmVm.sprite)
        {
            this->anmManager->Draw(&anmVm);
        }
    }

    void Error(const char *message)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, MAIN_WINDOW_TITLE, message, this->window);
        this->running = false;
    }

    void Release(void)
    {
        if (anmManager)
        {
            delete anmManager;
        }

        if (window)
        {
            SDL_DestroyWindow(window);
        }

        SDL_Quit();
    }
};

int main(int argc, char **argv)
{
    MainWindow mainWindow = {};

    mainWindow.Init();
    mainWindow.MainLoop();
    mainWindow.Release();

    return 0;
}
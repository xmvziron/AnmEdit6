#ifndef _GLFUNC_INC_
#define _GLFUNC_INC_

/* this is to refer ImGui to our GL function table */

#define glBindTexture               g_glFuncTable.glBindTexture
#define glBlendFunc                 g_glFuncTable.glBlendFunc
#define glColorPointer              g_glFuncTable.glColorPointer
#define glDeleteTextures            g_glFuncTable.glDeleteTextures
#define glDisable                   g_glFuncTable.glDisable
#define glDisableClientState        g_glFuncTable.glDisableClientState
#define glDrawElements              g_glFuncTable.glDrawElements
#define glEnable                    g_glFuncTable.glEnable
#define glEnableClientState         g_glFuncTable.glEnableClientState
#define glGenTextures               g_glFuncTable.glGenTextures
#define glGetIntegerv               g_glFuncTable.glGetIntegerv
#define glGetTexEnviv               g_glFuncTable.glGetTexEnviv
#define glLoadIdentity              g_glFuncTable.glLoadIdentity
#define glMatrixMode                g_glFuncTable.glMatrixMode
#define glOrtho                     g_glFuncTable.glOrtho
#define glPixelStorei               g_glFuncTable.glPixelStorei
#define glPolygonMode               g_glFuncTable.glPolygonMode
#define glPopAttrib                 g_glFuncTable.glPopAttrib
#define glPopMatrix                 g_glFuncTable.glPopMatrix
#define glPushAttrib                g_glFuncTable.glPushAttrib
#define glPushMatrix                g_glFuncTable.glPushMatrix
#define glScissor                   g_glFuncTable.glScissor
#define glShadeModel                g_glFuncTable.glShadeModel
#define glTexCoordPointer           g_glFuncTable.glTexCoordPointer
#define glTexEnvi                   g_glFuncTable.glTexEnvi
#define glTexImage2D                g_glFuncTable.glTexImage2D
#define glTexParameteri             g_glFuncTable.glTexParameteri
#define glTexSubImage2D             g_glFuncTable.glTexSubImage2D
#define glVertexPointer             g_glFuncTable.glVertexPointer
#define glViewport                  g_glFuncTable.glViewport

#endif
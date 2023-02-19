/***
 *
 *	Copyright (c) 2022, Bacontsu. All rights reserved
 *	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/
//
// waterrenderer.cpp
//
// in this file, we alter the rendering of normal water plane and modifies the texture
//


#include <filesystem>
#include "stdio.h"
#include "stdlib.h"

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include "cl_entity.h"
#include "com_model.h"
#include "triangleapi.h"

#include "PlatformHeaders.h"
#include "SDL2/SDL.h"
#include <gl/GL.h>


#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV 0x84F5
#endif // !GL_TEXTURE_RECTANGLE_NV

extern void R_SetupScreenStuff();
extern void R_ResetScreenStuff();

void DrawQuad(int width, int height, int ofsX, int ofsY)
{
	glTexCoord2f(ofsX, ofsY);
	glVertex3f(0, 1, -1);
	glTexCoord2f(ofsX, height + ofsY);
	glVertex3f(0, 0, -1);
	glTexCoord2f(width + ofsX, height + ofsY);
	glVertex3f(1, 0, -1);
	glTexCoord2f(width + ofsX, ofsY);
	glVertex3f(1, 1, -1);
}

void CPortalRenderer::Init()
{
	// create a load of blank pixels to create textures with
	unsigned char* pBlankTex = new unsigned char[ScreenWidth * ScreenHeight * 3];
	memset(pBlankTex, 0, ScreenWidth * ScreenHeight * 3);

	// Create the SCREEN-HOLDING TEXTURE
	glGenTextures(1, &portalPass_1);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, portalPass_1);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth, ScreenHeight, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

	// Create the SCREEN-HOLDING TEXTURE
	glGenTextures(1, &portalPass_2);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, portalPass_2);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth, ScreenHeight, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

	// Create the SCREEN-HOLDING TEXTURE
	glGenTextures(1, &screenpass);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, portalPass_2);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB8, ScreenWidth, ScreenHeight, 0, GL_RGB8, GL_UNSIGNED_BYTE, pBlankTex);

	// free the memory
	delete[] pBlankTex;
}

void CPortalRenderer::VidInit()
{

}

void CPortalRenderer::DrawPortal()
{
	R_SetupScreenStuff();

	// =========== FIRST PORTAL =========== 

	Vector angle = gEngfuncs.GetLocalPlayer()->curstate.angles;
	Vector forward;
	AngleVectors(angle, forward, nullptr, nullptr);
	Vector vecdir = gPortalRenderer.m_Portal1[0] - gEngfuncs.GetLocalPlayer()->curstate.origin;
	vecdir.z = 0;

	// World2Screen conversion
	float screen[2];
	gEngfuncs.pTriAPI->WorldToScreen(gPortalRenderer.m_Portal1[0], screen);
	int x = XPROJECT(screen[0]);
	int y = YPROJECT(screen[1]);

	float vertex[2];
	gEngfuncs.pTriAPI->WorldToScreen(gPortalRenderer.m_PortalVertex[0][0], vertex);
	int screenvertexX = XPROJECT(vertex[0]);
	int screenvertexY = YPROJECT(vertex[1]);

	Vector2D firstVertex(screenvertexX, screenvertexY);

	gEngfuncs.pTriAPI->WorldToScreen(gPortalRenderer.m_PortalVertex[0][1], vertex);
	screenvertexX = XPROJECT(vertex[0]);
	screenvertexY = YPROJECT(vertex[1]);

	Vector2D secondVertex(screenvertexX, screenvertexY);

	gEngfuncs.pTriAPI->WorldToScreen(gPortalRenderer.m_PortalVertex[0][2], vertex);
	screenvertexX = XPROJECT(vertex[0]);
	screenvertexY = YPROJECT(vertex[1]);

	Vector2D thirdVertex(screenvertexX, screenvertexY);

	int height = (int)Vector2D(firstVertex - thirdVertex).Length();
	int width = (int)Vector2D(firstVertex - secondVertex).Length();

	y = ScreenHeight/2;

	// check if its out from player's view
	if (DotProduct(forward, vecdir) > 45)
	{
		glViewport(x - width / 2, y - height / 2, width, height);
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, portalPass_1);
		glBegin(GL_QUADS);
		DrawQuad(width, height, x - width / 2, y - height / 2);
		glEnd();
	}

	// =========== SECOND PORTAL =========== 

	vecdir = gPortalRenderer.m_Portal2[0] - gEngfuncs.GetLocalPlayer()->curstate.origin;
	vecdir.z = 0;

	// World2Screen conversion
	gEngfuncs.pTriAPI->WorldToScreen(gPortalRenderer.m_Portal2[0], screen);
	x = XPROJECT(screen[0]);
	y = YPROJECT(screen[1]);

	gEngfuncs.pTriAPI->WorldToScreen(gPortalRenderer.m_PortalVertex[1][0], vertex);
	screenvertexX = XPROJECT(vertex[0]);
	screenvertexY = YPROJECT(vertex[1]);

	firstVertex = Vector2D(screenvertexX, screenvertexY);

	gEngfuncs.pTriAPI->WorldToScreen(gPortalRenderer.m_PortalVertex[1][1], vertex);
	screenvertexX = XPROJECT(vertex[0]);
	screenvertexY = YPROJECT(vertex[1]);

	secondVertex = Vector2D(screenvertexX, screenvertexY);

	gEngfuncs.pTriAPI->WorldToScreen(gPortalRenderer.m_PortalVertex[1][2], vertex);
	screenvertexX = XPROJECT(vertex[0]);
	screenvertexY = YPROJECT(vertex[1]);

	thirdVertex = Vector2D(screenvertexX, screenvertexY);

	height = (int)Vector2D(firstVertex - thirdVertex).Length();
	width = (int)Vector2D(firstVertex - secondVertex).Length();

	y = ScreenHeight / 2;

	//y = (ScreenHeight / 2) - y + height/2; // ???? what the fuck

	// check if its out from player's view
	if (DotProduct(forward, vecdir) > 45)
	{
		glViewport(x - width / 2, y - height / 2, width, height);
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, portalPass_2);
		glBegin(GL_QUADS);
		DrawQuad(width, height, x - width / 2, y - height / 2);
		glEnd();
	}

	glViewport(0, 0, ScreenWidth, ScreenHeight);

	R_ResetScreenStuff();
}

void CPortalRenderer::CapturePortalView(int pass)
{
	R_SetupScreenStuff();

	if (pass == 0)
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, portalPass_1);
	else if (pass == 1)
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, portalPass_2);
	else
		glBindTexture(GL_TEXTURE_RECTANGLE_NV, screenpass);

	glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, 0, 0, ScreenWidth, ScreenHeight, 0);
	R_ResetScreenStuff();
}
/// NOTEX: force OpenGL?
#if defined(WIN32) || defined(_WIN32)

// Prevent tons of unused windows definitions
#define WIN32_LEAN_AND_MEAN

#define OPENGL_AVAILABLE 1

//Disable all Windows 10 and older APIs otherwise pulled in by Windows.h
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

//Disable additional stuff not covered by the Windows.h list
#define NOWINRES
#define NOIME

#pragma push_macro("ARRAYSIZE")
#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif
#define HSPRITE WINDOWS_HSPRITE

#include <Windows.h>

#undef HSPRITE
#pragma pop_macro("ARRAYSIZE")
#else

#endif

#include <gl\GL.h>

#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV 0x84F5
#endif

#define MAX_MOTIONBLUR_FRAME 10
#define NUM_OF_CHANNELS 3
/// NOTEX: force OpenGL?


#include <cmath>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"
#include "screenfade.h"
#include "shake.h"

DECLARE_MESSAGE(m_BlurEffect, BlurEffect)

// NOTEZ: Source (https://github.com/bacontsu/codename-borea/blob/spirit/spirit-1.8-trinity-op4/cl_dll/blur.cpp)
void CBlurTexture::Init(int width, int height)
{
	// NOTEZ: 3 channels, RGB
	unsigned char* pBlankTexture = new unsigned char[width * height * NUM_OF_CHANNELS];

	memset(pBlankTexture, 0, width * height * NUM_OF_CHANNELS);

	glGenTextures(1, &m_uiTexture);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_uiTexture);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, width, height, 0, GL_RGBA8, GL_UNSIGNED_BYTE, 0);

	delete[] pBlankTexture;
}

void CBlurTexture::BindTexture(int width, int height)
{
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_uiTexture);
	glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA8, 0, 0, width, height, 0);
}

void CBlurTexture::DrawQuad(int width, int height)
{
	glTexCoord2f(0, 0);
	glVertex3f(0, 1, -1);

	glTexCoord2f(0, height);
	glVertex3f(0, 0, -1);

	glTexCoord2f(width, height);
	glVertex3f(1, 0, -1);

	glTexCoord2f(width, 0);
	glVertex3f(1, 1, -1);
}

void CBlurTexture::Draw(int width, int height)
{
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glColor3d(1, 1, 1);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0, 1, 1, 0, 0.1, 100);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, m_uiTexture);
	glColor4f(m_fRed, m_fGreen, m_fBlue, m_fAlpha);

	{
		glBegin(GL_QUADS);
		DrawQuad(width, height);
		glEnd();
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glDisable(GL_TEXTURE_RECTANGLE_NV);
	glEnable(GL_DEPTH_TEST);
}

int CBlurEffect::Init(void)
{
	gHUD.AddHudElem(this);
	HOOK_MESSAGE(BlurEffect);

	m_bIsBlurActive = false;
	m_flEffectEnd = 0;

	m_iBlurPos = 1;
	for (int i = 0; i < MAX_MOTIONBLUR_FRAME; i++)
	{
		m_pTextures[i].Init(ScreenWidth, ScreenHeight);
	}

	return 1;
}

CBlurEffect gBlur{};

int CBlurEffect::VidInit(void)
{
	for (int i = 0; i < MAX_MOTIONBLUR_FRAME; i++)
	{
		memset(&gBlur.m_pTextures[i], 0, sizeof(gBlur.m_pTextures));
	}
	m_fNextFrameUpdates = 0;

	return 1;
}

int CBlurEffect::Draw(float flTime)
{
	if (!m_bIsBlurActive || flTime > m_flEffectEnd)
	{
		m_bIsBlurActive = false;
		return 0;
	}

	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	glEnable(GL_BLEND);

	for (int i = 0; i < MAX_MOTIONBLUR_FRAME; i++)
	{
		m_pTextures[i].m_fRed = 1;
		m_pTextures[i].m_fGreen = 1;
		m_pTextures[i].m_fBlue = 1;
		m_pTextures[i].Draw(ScreenWidth, ScreenHeight);
		m_pTextures[i].m_fAlpha = 0.89f;
	}

	if (m_fNextFrameUpdates < gHUD.m_flTime)
	{
		m_pTextures[m_iFrameCounter].BindTexture(ScreenWidth, ScreenHeight);
		m_iFrameCounter++;

		if (m_iFrameCounter >= MAX_MOTIONBLUR_FRAME)
		{
			m_iFrameCounter = 0;
		}

		m_fNextFrameUpdates = gHUD.m_flTime + gHUD.m_flTimeDelta;
	}

	if (m_fNextFrameUpdates > gHUD.m_flTime + gHUD.m_flTimeDelta)
	{
		m_fNextFrameUpdates = 0;
	}

	glDisable(GL_BLEND);

	return 1;
}

void CBlurEffect::Reset(void)
{
	m_iFlags = 0;

	for (int i = 0; i < MAX_MOTIONBLUR_FRAME; i++)
	{
		memset(&gBlur.m_pTextures[i], 0, sizeof(gBlur.m_pTextures));
	}
	m_fNextFrameUpdates = 0;
	m_bIsBlurActive = false;
	m_flEffectEnd = 0;
}

int CBlurEffect::MsgFunc_BlurEffect(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	char blur_active = READ_CHAR();
	long duration = READ_LONG();

	if (blur_active == '1')
	{
		ShowEffect(true, duration);
	}
	else
	{
		ShowEffect(false, 0);
	}
	return 1;
}

void CBlurEffect::ShowEffect(bool activate_blur, long duration)
{
	m_iFlags = HUD_ACTIVE;
	m_bIsBlurActive = true;
	m_flEffectEnd = gHUD.m_flTime + duration;
}

bool CBlurEffect::AnimateNextFrame(int desiredFrameRate)
{
	return false;
}


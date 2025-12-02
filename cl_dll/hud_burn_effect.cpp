#include <cmath>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE(m_BurnEffect, BurnEffect)

int CHudBurnEffect::Init(void)
{
	gHUD.AddHudElem(this);
	HOOK_MESSAGE(BurnEffect);

	m_flEffectEnd = 0;
	m_flIntensity = 0;
	m_iFlags = 0;
	m_iTotalFrame = 9;
	m_fCurrentFrame = 0;
	m_fFrameRate = 9;

	return 1;
}

int CHudBurnEffect::VidInit(void)
{
	//m_hSprite = SPR_Load("sprites/custom/asep.spr");
	m_hSprite = SPR_Load("sprites/custom/burned_effect.spr");

	if (m_hSprite)
	{
		//ConsolePrint("CEK3: Smoke Sprite Exists!\n");
		m_rcSprite.left = 0;
		m_rcSprite.top = 0;
		m_rcSprite.right = SPR_Width(m_hSprite, 0);
		m_rcSprite.bottom = SPR_Height(m_hSprite, 0);
	}

	return 1;
}

void CHudBurnEffect::Reset(void)
{
	m_flEffectEnd = 0;
	m_flIntensity = 0;
	m_iFlags = 0;
	m_iTotalFrame = 9;
	m_fCurrentFrame = 0;
	m_fFrameRate = 9;
}

int CHudBurnEffect::Draw(float flTime)
{
	if (flTime < m_flEffectEnd && m_hSprite)
	{
		// Calculate fade out
		float timeLeft = m_flEffectEnd - flTime;
		float alpha = std::min(timeLeft, 1.0f);

		// Animate
		float deltaTime = gHUD.m_flTimeDelta;
		m_fCurrentFrame = m_fCurrentFrame + (m_fFrameRate * deltaTime);

		// Loop animation or clamp to last frame
		int currentFrame = (int)m_fCurrentFrame % m_iTotalFrame; // Loop
		// int currentFrame = min((int)m_fCurrentFrame, m_iTotalFrame - 1); // Play once

		// Scale factor - increase this to make sprite bigger
		float scale = 1.0f; // 2x bigger, adjust as needed

		// Calculate scaled dimensions
		int scaledWidth = (int)(m_rcSprite.right * scale);
		int scaledHeight = (int)(m_rcSprite.bottom * scale);

		// Center the scaled sprite
		int x = (ScreenWidth - scaledWidth) / 2;
		//int y = (ScreenHeight - scaledHeight) / 2;
		int y = (ScreenHeight - scaledHeight);

		// Create scaled rectangle
		wrect_t scaledRect{};
		scaledRect.left = 0;
		scaledRect.top = 0;
		scaledRect.right = scaledWidth;
		scaledRect.bottom = scaledHeight;
		SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
		SPR_DrawAdditive(m_fCurrentFrame, 0, y, &scaledRect);

		SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
		SPR_DrawAdditive(m_fCurrentFrame, m_rcSprite.right, y, &scaledRect);

		SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
		SPR_DrawAdditive(m_fCurrentFrame, m_rcSprite.right * 2, y, &scaledRect);

		SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
		SPR_DrawAdditive(m_fCurrentFrame, m_rcSprite.right * 3, y, &scaledRect);

		SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
		SPR_DrawAdditive(m_fCurrentFrame, m_rcSprite.right * 4, y, &scaledRect);

		SPR_Set(m_hSprite, (int)(255 * alpha), (int)(105 * alpha), (int)(255 * alpha));
		SPR_DrawAdditive(m_fCurrentFrame, ScreenWidth - m_rcSprite.right, y, &scaledRect);

		//ConsolePrint("CEK3: Draw ");

		return 1;
	}
	return 0;
}

void CHudBurnEffect::ShowEffect(long duration, long intensity)
{
	m_flEffectEnd = gHUD.m_flTime + duration;
	m_flIntensity = intensity;
	m_iFlags = HUD_ACTIVE;
	m_fCurrentFrame = 0;
	//ConsolePrint("CEK3: Show Effect 1\n");
}

int CHudBurnEffect::MsgFunc_BurnEffect(const char* pszName, int iSize, void* pbuf)
{
	ConsolePrint("CEK3: Show Effect 0\n");
	BEGIN_READ(pbuf, iSize);

	long duration = READ_LONG();
	long intensity = READ_LONG();

	ShowEffect(duration, intensity);

	return 1;
}
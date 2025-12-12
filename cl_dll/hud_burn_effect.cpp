#include <cmath>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"

DECLARE_MESSAGE(m_BurnEffect, BurnEffect)

int CHudBurnEffect::Init(void)
{
	gHUD.AddHudElem(this);
	HOOK_MESSAGE(BurnEffect);

	m_flEffectEnd = 0;
	m_flIntensity = 0;
	m_iFlags = 0;
	m_iTotalFrame = 8;
	m_fCurrentFrame = 0.0f;
	m_fFrameRate = 8.0f;

	return 1;
}

int CHudBurnEffect::VidInit(void)
{
	m_hSprite = SPR_Load("sprites/custom/burned_effect.spr");
	//m_hBlurSprite = SPR_Load("sprites/white.spr");

	if (m_hSprite)
	{
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
	m_iTotalFrame = 8;
	m_fCurrentFrame = 0.0f;
	m_fFrameRate = 8.0f;
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
		m_fCurrentFrame += m_fFrameRate * deltaTime;

		// Loop animation
		int currentFrame = (int)m_fCurrentFrame % m_iTotalFrame;
		//// Play once
		//// int currentFrame = min((int)m_fCurrentFrame, m_iTotalFrame - 1);

		// Scale factor
		float scale = 1.0f;

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

		if (currentFrame <= m_iTotalFrame)
		{
			SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
			SPR_DrawAdditive(currentFrame, 0, y, &scaledRect);

			SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
			SPR_DrawAdditive(currentFrame, m_rcSprite.right, y, &scaledRect);

			SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
			SPR_DrawAdditive(currentFrame, m_rcSprite.right * 2, y, &scaledRect);

			SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
			SPR_DrawAdditive(currentFrame, m_rcSprite.right * 3, y, &scaledRect);

			SPR_Set(m_hSprite, (int)(255 * alpha), (int)(255 * alpha), (int)(255 * alpha));
			SPR_DrawAdditive(currentFrame, m_rcSprite.right * 4, y, &scaledRect);

			SPR_Set(m_hSprite, (int)(255 * alpha), (int)(105 * alpha), (int)(255 * alpha));
			SPR_DrawAdditive(currentFrame, ScreenWidth - m_rcSprite.right, y, &scaledRect);
		}

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
}

int CHudBurnEffect::MsgFunc_BurnEffect(const char* pszName, int iSize, void* pbuf)
{
	ConsolePrint("CEK3: Show Burn Effect\n");
	BEGIN_READ(pbuf, iSize);

	long duration = READ_LONG();
	long intensity = READ_LONG();

	ShowEffect(duration, intensity);

	return 1;
}
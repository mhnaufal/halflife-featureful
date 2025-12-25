/***
*
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "weapons.h"

#define	PURPLE_GRENADE_PRIMARY_VOLUME		450

// NOTEX: based on the .mdl file, animation sequence number
enum purplegrenade_e
{
	PURPLE_GRENADE_IDLE = 0,
	PURPLE_GRENADE_PULLPIN,
	PURPLE_GRENADE_THROW,
	PURPLE_GRENADE_DEPLOY
};

class CPurpleGrenade : public CConfigurableWeapon
{
public:
	int WeaponId() const override { return WEAPON_PURPLE_GRENADE; }
	bool GetItemInfo(ItemInfo* p) override;
	WeaponParameters GetDefaultParameters() const override;

	void PrimaryAttack() override;
	bool Deploy() override;
	bool CanHolster() override;
	void Holster() override;
	void WeaponIdle() override;
	bool PreferNewPhysics();

	void GetWeaponData(weapon_data_t& data) override;
	void SetWeaponData(const weapon_data_t& data) override;
};

LINK_WEAPON_TO_CLASS( weapon_purplegrenade, CPurpleGrenade )

bool CPurpleGrenade::GetItemInfo( ItemInfo *p )
{
	p->pszName = "purple grenade";
	p->iId = WEAPON_PURPLE_GRENADE;
	p->iSlot = 5;
	p->iPosition = 2;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->pszAmmoEntity = STRING(pev->classname);
	p->iDropAmmo = MyParameters().initialAmmoAmount.min;

	return true;
}

WeaponParameters CPurpleGrenade::GetDefaultParameters() const
{
	WeaponParameters params;

	params.initialAmmoAmount = PURPLE_GRENADE_MAX_CARRY;
	params.maxClip = WEAPON_NOCLIP;
	params.ammoName = "Purple Grenade";

	params.worldModel = "models/w_smokegrenade.mdl";
	params.viewModel = "models/v_smokegrenade.mdl";
	params.playerModel = "models/p_smokegrenade.mdl";
	params.playerAnimExt = "crowbar";
	params.priority = 6;

	params.deploy.animIndex = PURPLE_GRENADE_DEPLOY;

	params.idleAnims.main = WeaponParameters::IdleAnimArray{
		WeaponParameters::IdleAnim{PURPLE_GRENADE_IDLE, 0.75f, FloatRange(10.0f, 15.0f)},
		WeaponParameters::IdleAnim{PURPLE_GRENADE_PULLPIN, 0.25f, FloatRange(75.0f / 30.0f)},
	};

	params.fire.fireType = WeaponParameters::Fire::PROJECTILE;
	params.fire.projectileName = "purple grenade";
	params.fire.projectileOffsetForward = 5.0f;
	params.fire.allowUnderwater = true;

	return params;
}

bool CPurpleGrenade::Deploy()
{
	m_flReleaseThrow = -1;
	ALERT(at_console, "[RECON] Purple Hallucination Grenade grabbed.\n");
	return PerformDeploy();
}

bool CPurpleGrenade::CanHolster()
{
	// can only holster purple grenades when not primed!
	return ( m_flStartThrow == 0 );
}

void CPurpleGrenade::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

	if( HasAmmoToFire() )
	{
		SendWeaponAnim( PURPLE_GRENADE_DEPLOY );
	}
	else
	{
		// no more grenades!
		m_pPlayer->ClearWeaponBit( WEAPON_PURPLE_GRENADE );
		DestroyItem();
	}

	if( m_flStartThrow )
	{
		m_flStartThrow = 0.0f;
		m_flReleaseThrow = 0.0f;
	}

	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "common/bodydrop1.wav", 2.0f, ATTN_NORM );
}

void CPurpleGrenade::PrimaryAttack()
{
	if( !m_flStartThrow && HasAmmoToFire() )
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0.0f;

		SendWeaponAnim( PURPLE_GRENADE_DEPLOY );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
	}
}

bool CPurpleGrenade::PreferNewPhysics()
{
	#if CLIENT_DLL
	extern cvar_t *cl_grenadephysics;
	if (cl_grenadephysics)
		return (int)cl_grenadephysics->value != 0;
	return false;
	#else
	if (m_pPlayer)
		return m_pPlayer->m_iPreferNewGrenadePhysics != 0;
	return false;
	#endif
}

void CPurpleGrenade::WeaponIdle()
{
	if( m_flReleaseThrow == 0.0f && m_flStartThrow )
		m_flReleaseThrow = gpGlobals->time;

	if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if( m_flStartThrow )
	{
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if( angThrow.x < 0.0f )
			angThrow.x = -10.0f + angThrow.x * ( ( 90.0f - 10.0f ) / 90.0f );
		else
			angThrow.x = -10.0f + angThrow.x * ( ( 90.0f + 10.0f ) / 90.0f );

		float maxVel = 500.0f;
		float velVultiplier = 4.0f;
		if (PreferNewPhysics())
		{
			maxVel = 1000.0f;
			velVultiplier = 6.5f;
		}

		float flVel = ( 90.0f - angThrow.x ) * velVultiplier;
		if( flVel > maxVel )
			flVel = maxVel;

		UTIL_MakeVectors( angThrow );

		// alway explode 3 seconds after the pin was pulled
		float time = m_flStartThrow - gpGlobals->time + 3.0f;
		if( time < 0.0f )
			time = 0.0f;

#if !CLIENT_DLL
		const Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16.0f;
		const Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		CGrenade::ShootTimed(m_pPlayer, vecSrc, vecThrow, time, EntityOverrides{}, GRENADE_TYPE::PURPLE_GRENADE);
#endif

		SendWeaponAnim( PURPLE_GRENADE_THROW );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		//m_flReleaseThrow = 0.0f;
		m_flStartThrow = 0.0f;
		m_flNextPrimaryAttack = GetNextAttackDelay( 0.5f );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;

		SpendAmmo();

		if( !HasAmmoToFire() )
		{
			// just threw last grenade
			// set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay( 0.5f );// ensure that the animation can finish playing
		}
		return;
	}
	else if( m_flReleaseThrow > 0.0f )
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0.0f;

		if( HasAmmoToFire() )
		{
			SendWeaponAnim( PURPLE_GRENADE_DEPLOY );
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0f, 15.0f );
		m_flReleaseThrow = -1.0f;
		return;
	}

	if( HasAmmoToFire() )
	{
		SendIdleAnimation();
	}
}

void CPurpleGrenade::GetWeaponData(weapon_data_t& data)
{
	data.fuser2 = m_flStartThrow;
	data.fuser3 = m_flReleaseThrow;
}
void CPurpleGrenade::SetWeaponData(const weapon_data_t& data)
{
	m_flStartThrow = data.fuser2;
	m_flReleaseThrow = data.fuser3;
}

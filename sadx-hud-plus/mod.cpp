#include "pch.h"
#include "SADXModLoader.h"
#include "IniFile.hpp"
#include "sadx.h"
#include "sadx-utils.h"

using namespace uiscale;

static const HelperFunctions* gHelperFunctions;

static bool boolShowScore = false;
static bool boolRemoveLimits = false;
static bool boolSuperSonic = false;

void DrawScore(Float& y)
{
	titleSprite.p.x = 7.5f;
	titleSprite.p.y = y - 10.0f;
	late_DrawSprite2D(&titleSprite, 0, 22046.498f, NJD_SPRITE_ALPHA, LATE_LIG);

	auto score = ssStageNumber == STAGE_SANDBOARD ? slScore : slEnemyScore;

	NJS_SPRITE _sp;
	_sp.p.x = 190.0f;
	_sp.p.y = y - 1.5f;
	_sp.tlist = &texlist_score;
	_sp.tanim = &anim_score;
	_sp.sx = 1.0f;
	_sp.sy = 1.0f;

	for (int i = 0; i < 8; ++i)
	{
		if (score <= 0)
		{
			SetMaterial(0.8f, 0.8f, 0.8f, 0.8f);
			njDrawSprite2D_ForcePriority(&_sp, 0, 0, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR);
		}
		else
		{
			njDrawSprite2D_ForcePriority(&_sp, score % 10, -1.5f, NJD_SPRITE_ALPHA);
			score /= 10;
		}

		_sp.p.x -= 16.0f;
	}

	ResetMaterial();

	y += 16;
}

void DrawTimer(Float& y)
{
	sprite_score.p.x = 16.0f;
	sprite_score.p.y = y;
	njDrawSprite2D_ForcePriority(&sprite_score, TEX_CON_HYOUJI, -1.501f, NJD_SPRITE_ALPHA);

	DrawTimeSprite(64.0f, y + 1.0f);

	y += 16;
}

void DrawRings(Float& y)
{
	sprRing.p.x = 16.0f;
	sprRing.p.y = y + 1.5f;
	njDrawSprite2D_ForcePriority(&sprRing, 0, -1.501f, NJD_SPRITE_ALPHA);

	auto rings = GetNumRing();
	if (rings < 0)
	{
		rings = 0;
	}

	float color;
	if (rings)
	{
		color = 1.0f;
	}
	else
	{
		if (!ChkPause())
		{
			ang += 0x400;
		}

		auto sin = njSin(ang);
		color = 1.0f - sin * sin;
	}

	sprite_score.p.x = 42.0f;
	sprite_score.p.y = y + 4.0f;
	SetMaterial(1.0f, 1.0f, color, color);
	DisplaySNumbers(&sprite_score, rings, boolRemoveLimits ? max(3, (int)log10(rings) + 1) : 3);
	ResetMaterial();

	y += 16;
}

static NJS_TEXNAME SUPERSONIC_EXTRA_TEXNAME[6];
NJS_TEXLIST SUPERSONIC_EXTRA_TEXLIST = { arrayptrandlength(SUPERSONIC_EXTRA_TEXNAME) };

enum SUPERSONIC_EXTRA
{
	SUPERSONIC_EXTRA_LIFE,
	SUPERSONIC_EXTRA_1UP
};

static NJS_TEXANIM SUPERSONIC_EXTRA_TEXANIM[2] = {
	{ 0x20, 0x20, 0, 0, 0, 0, 0x100, 0x100, SUPERSONIC_EXTRA_LIFE, 0x20 },
	{ 0x20, 0x20, 0x10, 0x10, 0, 0, 0xFF, 0xFF, SUPERSONIC_EXTRA_1UP, 0x20 }
};

static NJS_SPRITE SUPERSONIC_EXTRA_SPRITE = { {}, 1.0f, 1.0f, 0, &SUPERSONIC_EXTRA_TEXLIST, SUPERSONIC_EXTRA_TEXANIM };

bool IsSuperSonic(playerwk* pwp)
{
	return (pwp->equipment & Upgrades_SuperSonic);
}

bool IsSuperSonic(int pnum)
{
	return playerpwp[pnum] && IsSuperSonic(playerpwp[pnum]);
}

void DrawLives()
{
	auto player = playertwp[0];
	auto pwp = playerpwp[0];
	gHelperFunctions->PushScaleUI(Align::Align_Bottom, false, 1.0f, 1.0f);

	sprite_score.p.x = 16.0f;
	sprite_score.p.y = ScreenRaitoY * 480.0f - 64.0f;

	if (!boolSuperSonic)
	{
		njSetTexture(&CON_REGULAR_TEXLIST);

		njDrawSprite2D_ForcePriority(&sprite_score, gu8flgPlayingMetalSonic ? 24 : GetPlayerNumber() + TEX_CON_ZANKI, -1.501f, NJD_SPRITE_ALPHA);
	}
	else
	{
		if ((player && TWP_CHAR(player) == Characters_Sonic) && (pwp && IsSuperSonic(pwp)))
		{
			njSetTexture(&SUPERSONIC_EXTRA_TEXLIST);

			SUPERSONIC_EXTRA_SPRITE.p.x = sprite_score.p.x;
			SUPERSONIC_EXTRA_SPRITE.p.y = sprite_score.p.y;
			njDrawSprite2D_ForcePriority(&SUPERSONIC_EXTRA_SPRITE, SUPERSONIC_EXTRA_LIFE, -1.501f, NJD_SPRITE_ALPHA);
		}
		else
		{
			njSetTexture(&CON_REGULAR_TEXLIST);

			njDrawSprite2D_ForcePriority(&sprite_score, gu8flgPlayingMetalSonic ? 24 : GetPlayerNumber() + TEX_CON_ZANKI, -1.501f, NJD_SPRITE_ALPHA);
		}
	}

	sprite_score.p.x = 49.0f;
	sprite_score.p.y += 8.0f;
	SetMaterial(1.0f, 1.0f, 1.0f, 1.0f);

	auto lives = (Sint16)GetNumPlayer();
	if (score_display >= 0)
	{
		DisplaySNumbers(&sprite_score, lives, boolRemoveLimits ? max(2, (int)log10(lives) + 1) : 2);
	}

	ResetMaterial();

	gHelperFunctions->PopScaleUI();
}

bool IsBossLevel()
{
	return ssStageNumber >= STAGE_CHAOS0 && ssStageNumber <= STAGE_E101_R;
}

void DisplayScoreAction_r()
{
	ghDefaultBlendingMode();

	sprite_score.sy = 1.0f;
	sprite_score.sx = 1.0f;
	
	if (score_u_display >= 0)
	{
		gHelperFunctions->PushScaleUI(Align::Align_Default, false, 1.0f, 1.0f);

		Float y = 32;

		if (GetLevelType() == STG_TYPE_ADVENTURE)
		{
			DrawRings(y);
		}
		else if (IsBossLevel())
		{
			DrawTimer(y);
			DrawRings(y);
		}
		else
		{
			if (boolShowScore)
			{
				DrawScore(y);
			}

			DrawTimer(y);

			// No rings in Sand Hill
			if (ssStageNumber != STAGE_SANDBOARD)
			{
				DrawRings(y);
			}
		}

		gHelperFunctions->PopScaleUI();
	}

	if (score_d_display >= 0)
	{
		DrawLives();
	}
}

void __cdecl DisplayScore_r()
{
	if (score_display < 0 || GetPlayerNumber() == Characters_Big || ssStageNumber == STAGE_MG_CART)
	{
		return;
	}

	DisplayScoreAction_r();

	// If in adventure field, hide hud elements
	if (GetLevelType() == STG_TYPE_ADVENTURE)
	{
		score_d_display = -1; // lives
		score_u_display = -1; // timer
	}
	else
	{
		score_d_display = score_display; // lives
		score_u_display = score_display; // timer
	}
}

void __cdecl ExtraDisplayPause(task* tp)
{
	// Making sure we're paused
	if (ChkPause())
	{
		auto twp = tp->twp;
		if (twp->ang.x == 0)
			twp->ang.x = 0x4000; // Instant spawn minimals unless it's already moving
		twp->wtimer = 150ui16; // Trigger disappearance as soon as pause is disabled
		twp->mode = 1i8; // Active mode
		tp->exec(tp);
	}
}

void __cdecl ExtraDisplayInit_r()
{
	auto tp = CreateElementalTask(2u, 2, ExtraDisplay);
	tp->disp = ExtraDisplayPause; // Add a display to run in pause menu
}

extern "C"
{
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		
		boolShowScore = config->getBool("", "ShowScore", true);
		boolRemoveLimits = config->getBool("", "RemoveLimits", true);
		boolSuperSonic = config->getBool("", "SuperSonicCompatibility", false);

		gHelperFunctions = &helperFunctions;

		WriteJump((void*)0x425F90, DisplayScore_r); // Hook DisplayScore, which draws the HUD
		WriteData((uint8_t*)0x427F50, 0xC3ui8); // Remove DisplayTimer, which draws the timer/score digits

		// Sprite fixes:
		anim_score[TEX_CON_HYOUJI].sy = 17;
		anim_score[TEX_CON_HYOUJI].v2 = 68;
		titleAnim.sy = 28;
		aniRing.v1 = 71;

		if (boolShowScore)
		{
			helperFunctions.RegisterCommonObjectPVM({ "BOARD_SCORE", &BOARD_SCORE_TEXLIST }); // Force score texture to always be loaded
		}

		if (config->getBool("", "ShowAnimalsPause", true))
		{
			WriteJump((void*)0x46B650, ExtraDisplayInit_r); // Add pause display to minimal task
		}
		
		if (config->getBool("", "SuperSonicCompatibility", true))
		{
			helperFunctions.RegisterCharacterPVM(Characters_Sonic, { "SUPERSONIC_EXTRA", &SUPERSONIC_EXTRA_TEXLIST });
		}
		
		delete config;
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}

#include "W2S.h"
#include "ESP.h"
#include "StepMania.h"
#include "Menu.h"
#include "PlayerList.h"

void ASUS();

void __fastcall hkPaintTraverse(void* pThis, void*, unsigned int Panel, bool allowForce, bool forceRepaint) {
	PanelVmt->getOrig<void(__thiscall*)(void*, unsigned int, bool, bool)>(41)(pThis, Panel, allowForce, forceRepaint);

	static unsigned int overlayPanel;

	if (!overlayPanel)
		if (strstr(Interfaces->Panel->GetName(Panel), "MatSystemTopPanel"))
			overlayPanel = Panel;

	if (!overlayPanel || overlayPanel != Panel) return;

//	if (!Stepmania::bHasPrecached) Stepmania::Precache();


	/*CBaseEntity* pLocal = Interfaces->EntList->GetClientEntity(Interfaces->Engine->GetLocalPlayer());
	if (pLocal && Menu::ScrW) {
		CViewSetup pViewSetup;
		if (Interfaces->Client->GetViewSetup(pViewSetup)) {
			pViewSetup.x = Menu::ScrW;
			pViewSetup.y = Menu::ScrH;
			pViewSetup.origin = pLocal->AbsOrigin() + pLocal->m_vecViewOffset();
			pViewSetup.angles = Vector(0, 0, 0);
			pViewSetup.angles.y += 180.0f; //rotate yaw
			pViewSetup.fov = 90.0f; //default fov
			Interfaces->Client->RenderView(pViewSetup, 0, 2); //render our view on the screen
		}
	}*/

	ESP::Draw();
	Menu::Draw();

	ASUS();

	Resolver::UpdatePlayers();

	if (!Interfaces->Engine->IsInGame() || !Interfaces->Engine->IsConnected())

		KillSounds::Update(false, true);

	static float TimeForNext[65];
	
	for (int i = 1; i < 65; i++) {
		TimeForNext[i] -= Interfaces->Globals->frametime;
		
		if (TimeForNext[i] <= 0.f) {
			TimeForNext[i] = 0.1f;
			CurrentStage[i]++;
		}

		if (CurrentStage[i] >= 8)
			CurrentStage[i] = 0;
	}

	//if(Stepmania::bDrawOverlay) Stepmania::DrawOverlay();

	/*if (!Stepmania::bDrawOverlay) {
		if (GetAsyncKeyState(VK_F6) & 0x1) {
			Stepmania::ConfirmSelection();
		}
	}*/
}
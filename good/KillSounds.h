namespace KillSounds {
	int Kills;
	void PlayKillSound(int);
}

void KillSounds::Update(bool bAddKill, bool bReset) {
	if (bAddKill)
		Kills++;
	else if (bReset)
		Kills = 0;

	PlayKillSound(Kills % 15);
}

const char* pSounds[] = {
	"C:\\CSGOSound\\",
};

void KillSounds::PlayKillSound(int KillNumber) {
	const char* KillSound = pSounds[rand() % (sizeof(pSounds) / 4)];
}
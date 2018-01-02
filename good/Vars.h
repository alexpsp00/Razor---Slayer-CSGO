struct Vars {
	struct AntiAim {
		struct Normal{
			int Enabled;
			int Yaw;
			int Pitch;
			int YawMode;
			int TargetMode = 2;
		} Normal;
		struct Duck {
			int Enabled;
			int Yaw;
			int Pitch;
			int YawMode;
			int TargetMode = 2;
		} Duck;
		struct Wall {
			int Enabled;
			int Pitch;
			int YawMode;
		} Wall;
	} AntiAim;
	struct Fakelag {
		int Enabled = 0;
		int Mode;
		int Factor = 12;
	} Fakelag;
	struct Visuals {
		struct Player {
			struct Chams {
				int Enabled = 1;
				float Alpha = 1.f;
				int ViewModelChams = 1;
				int HandChams = 1;
				int Rainbow;
			} Chams;
			int Box = 1;
			int Name = 1;
			int Weapon = 1;
			int Health = 1;
			int EnemyOnly;
			int Aimspot;
			int Outline = 3;
		} Player;
		struct World {
			float ASUS = .6f;
		} World;
	} Visuals;
	struct Aimbot {
		struct Accuracy {
			int LagFix = 2;
			int NoRecoil = 1;
			int Resolver;
		} Accuracy;
		int Enabled = 1;
		int MultiSpot = 0;
		int MultiBox = 0;
		int Hitscan;
		int Bodyaim;
		int AWPBody = 1;
		int Silent = 2;
		int AutoShoot = 1;
		int Autowall = 1;
		int TargetSelection = 0;
		int Aimspot = 0;
		float PointScale = .05f;
	} Aimbot;
	struct Misc {
		int RoundSay;
		int Bunnyhop = 1;
	} Misc;
} Vars;
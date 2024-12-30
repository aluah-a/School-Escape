#pragma once

class CMyGame : public CGame
{
	CSprite m_player;				// Player
	CVector m_dest;					// Player destination point

	CSprite cursor;
	bool clicked = false;
	bool draggingPrisoner = false;
	bool draggingPillow = false;
	bool draggingOnion = false;
	bool prisonerAttached = false;
	bool pillowAttached = false;
	bool onionAttached = false;

	bool question1 = false;
	bool question2 = false;
	bool question3 = false;

	bool unlocked = false;
	bool gamewon = false;

	CSprite doorway, doorway1, table1, table2;

	CSprite line1, line11, line12;
	CSprite line2, line3;

	bool speechTextEnter = false;

	CSprite spark1, spark2, spark3;
	CSprite prisonerHint, pillowHint, onionHint;
	CSprite prisonerHint1, pillowHint1, onionHint1;
	CSprite prisoner, pillow, onion;

	CSprite paper1, paper2, paper1triggerbox, paper2triggerbox, paper3triggerbox;
	CSprite Ebutton, Ebutton1;
	CSprite triggerBox, riddleBox1, riddleBox2, riddleBox3;
	CSprite finish;

	CSprite riddleMenu, backButton, backButton1;
	bool riddleMenuEntered = false;


	CSprite darkScreen, darkScreen1;

	CSprite guardSpeechSceen;
	CSprite playerSpeechSceen;

	int chatCD = -1;

	int textStart = 30;
	int text1cd = -1;
	int text2cd = -1;
	int text3cd = -1;
	int text4cd = -1;
	int text5cd = -1;

	bool text1 = false;;
	bool text2 = false;
	bool text3 = false;
	bool text4bad = false;
	bool text4 = false;
	bool text5 = false;
	bool text5bad = false;

	bool goodChoice = false;
	bool badChoice = false;
	bool okChoice = false;

	bool speechSceneExit = false;
	int speechSceneExitCD = -1;


	CSprite playerChat, securityChat;

	CSprite background, background1;

	CSpriteVector m_guards;			// Enemies
	CSprite* m_pKiller;				// Killer enemy - positive sight test!

	CSprite startSpeechSceneBG;
	CSprite speechSceneEnter1, speechSceneEnter2, speechSceneEnter3, speechSceneEnter4;
	CSprite speechSceneBG;
	CSprite speechSceneStandPlayer, speechSceneStandSecurity;

	CSprite textmenubox;
	CSprite textbox1, textbox2, textbox3;

	bool speech1 = false;
	bool speech2 = false;
	bool speech3 = false;

	bool highlighted1 = false;
	bool highlighted2 = false;
	bool highlighted3 = false;
	bool highlighted4 = false;
	bool highlighted5 = false;
	bool highlighted6 = false;
	bool highlighted7 = false;
	bool highlighted8 = false;
	bool highlighted9 = false;

	int whichSecurity = 0;

	bool startSpeechScene = false;
	bool speechScene = false;
	bool speechSceneEnter = false;
	bool speechSceneCharEnter = false; //Char = character
	bool enter = false;
	int startspeechsceneCD = 100; // CD = cooldown
	int speechsceneCD = 50; // CD = cooldown
	int speechsceneCharCD = 300; // CD = cooldown

	bool securityPass = false;
	int securityMemoryLossCD = 0; //after cooldown security will again trigger speech scene if the player is in sight

	bool playerFaceL = false;
	bool playerFaceR = false;
	bool playerFaceU = false;

	int speechSceneEnterTimes = 0;

	int sparkRotation = 0;

	bool prisonerCollected = false;
	bool pillowCollected = false;
	bool onionCollected = false;
	int pressedEcounter = 0;


	CSpriteList m_tiles;			// Tiles
	static char *m_tileLayout[35];	// Tiles layout

public:
	CMyGame(void);
	~CMyGame(void);

	// Per-Frame Callback Funtions (must be implemented!)
	virtual void OnUpdate();
	virtual void OnDraw(CGraphics* g);

	// Game Life Cycle
	virtual void SpeechText1();
	virtual void SpeechText2();
	virtual void SpeechText3();

	virtual void SpeachSceneEnter();
	virtual void SpeachSceneExit();

	virtual void CollectHints();
	virtual void OpenRiddleDoor();

	virtual void OnInitialize();
	virtual void OnDisplayMenu();
	virtual void OnStartGame();
	virtual void OnStartLevel(Sint16 nLevel);
	virtual void OnGameOver();
	virtual void OnTerminate();

	// Keyboard Event Handlers
	virtual void OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode);
	virtual void OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode);

	// Mouse Events Handlers
	virtual void OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle);
	virtual void OnLButtonDown(Uint16 x,Uint16 y);
	virtual void OnLButtonUp(Uint16 x,Uint16 y);
	virtual void OnRButtonDown(Uint16 x,Uint16 y);
	virtual void OnRButtonUp(Uint16 x,Uint16 y);
	virtual void OnMButtonDown(Uint16 x,Uint16 y);
	virtual void OnMButtonUp(Uint16 x,Uint16 y);
};

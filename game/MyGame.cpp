#include "stdafx.h"
#include "MyGame.h"

#pragma warning (disable: 4244)

#define SPEED_PLAYER	100

#define SPEED_GUARD_0	120
#define SPEED_GUARD_1	80
#define SPEED_GUARD_2	80

char* CMyGame::m_tileLayout[35] =
{
	"XXXXXXXXXXXXXXXXXXXXXXXXXX",
	"X                        X",
	"X                        X",
	"X                        X",
	"X XXXXXXXXXXXXXXXXX XXXXXX",
	"X XXXXXXXXXXXXXXXXX XXXXXX",
	"X XXXXXXXXXXXXXXXXX XXXXXX",
	"X  XXX  XXXX   X X   X X X",
	"X          X   X X   X X X",
	"X  XXX  XXXX   X X   X X X",
	"X  XXX  XXXX             X",
	"X          X   X X   X X X",
	"X          X   X X   X X X",
	"XXXXXXXXXXXXX XXXXXXXXXXXX",
	"             X            ",
};

CMyGame::CMyGame(void) :
	m_player(0, 0, "boy.png", 0)
{
	m_pKiller = NULL;
}

CMyGame::~CMyGame(void)
{
}

// MATHS! Intersection function
// Provides information about the interception point between the line segments: a-b  and c-d
// Returns true if the lines intersect (not necessarily  within the segments); false if they are parallel
// k1 (returned value): position of the intersection point along a-b direction:
//                      k1==0: at the point a; 0>k1>1: between a and b; k1==1: at b; k1<0 beyond a; k1>1: beyond b
// k2 (returned value): position of the intersection point along c-d direction
//                      k2==0: at the point c; 0>k2>1: between c and d; k2==1: at d; k2<0 beyond c; k2>1: beyond d
// Intersection point can be found as X = a + k1 * (b - a) = c + k2 (d - c)
bool Intersection(CVector a, CVector b, CVector c, CVector d, float &k1, float &k2)
{
	CVector v1 = b - a;
	CVector v2 = d - c;
	CVector con = c - a;
	float det = v1.m_x * v2.m_y - v1.m_y * v2.m_x;
	if (det != 0)
	{
		k1 = (v2.m_y * con.m_x - v2.m_x * con.m_y) / det;
		k2 = (v1.m_y * con.m_x - v1.m_x * con.m_y) / det;
		return true;
	}
	else
		return false;
}

// returns true is the line segments a-b and c-d intersect
bool Intersection(CVector a, CVector b, CVector c, CVector d)
{
	float k1, k2;
	if (!Intersection(a, b, c, d, k1, k2))
		return false;
	return k1 >= 0 && k1 <= 1.f && k2 >= 0 && k2 <= 1.f;
}


/////////////////////////////////////////////////////
// Per-Frame Callback Funtions (must be implemented!)

void CMyGame::OnUpdate()
{
	if (!IsGameMode()) return;

	Uint32 t = GetTime();

	//SPEECH SCENE ENTER CONTROL
	SpeachSceneEnter();

	//SPEECH SCENE EXIT
	SpeachSceneExit();

	//COLLECT HINTS
	CollectHints();

	//OPEN RIDDLE DOOR
	OpenRiddleDoor();


	if (m_player.HitTest(&line1) || m_player.HitTest(&line11) || m_player.HitTest(&line12))
	{
		whichSecurity = 1;
	}

	if (m_player.HitTest(&line2))
	{
		whichSecurity = 2;
	}

	if (m_player.HitTest(&line3))
	{
		whichSecurity = 3;
	}


	if (speechTextEnter && chatCD == 0)
	{
		if (whichSecurity == 1)
		{
			SpeechText1();
		}
		if (whichSecurity == 2)
		{
			SpeechText2();
		}
		if (whichSecurity == 3)
		{
			SpeechText3();
		}
	}

	if (chatCD > 0)
	{
		chatCD--;
	}


	// Player Control:a
	// The Player movement is contrained to the tile system.
	// Whilst moving, the player always heads towards the destination point at the centre of one of the neighbouring tiles - stored as m_dest.
	// Player control only activates when player either motionless or passed across the destination point (dest)

	if (!speechScene)
	{
		if (m_player.GetSpeed() < 0.1 || m_player.GetSpeed() > 0.1 && Dot(m_dest - m_player.GetPosition(), m_player.GetVelocity()) < 0)
		{
			CVector newDir(0, 0);		// new direction - according to the keyboard input
			char* newAnim = "idleL";		// new animation - according to the keyboard input

			if(playerFaceL)
			{
				newAnim = "idleL";
			}
			else if (playerFaceR)
			{
				newAnim = "idleR";
			}
			else if (playerFaceU)
			{
				newAnim = "idleU";
			}


			if (IsKeyDown(SDLK_a))
			{
				newDir = CVector(-1, 0);
				newAnim = "walkL";

				playerFaceL = true;
				playerFaceR = false;
				playerFaceU = false;
			}
			else if (IsKeyDown(SDLK_d))
			{
				newDir = CVector(1, 0);
				newAnim = "walkR";

				playerFaceL = false;
				playerFaceR = true;
				playerFaceU = false;
			}
			else if (IsKeyDown(SDLK_w))
			{
				newDir = CVector(0, 1);
				newAnim = "walkU";

				playerFaceL = false;
				playerFaceR = false;
				playerFaceU = true;
			}
			else if (IsKeyDown(SDLK_s))
			{
				newDir = CVector(0, -1);
				newAnim = "walkD";

				playerFaceL = true;
				playerFaceR = false;
				playerFaceU = false;
			}

			// Collision test of the new heading point
			CVector newDest = m_dest + 64 * newDir;
			for (CSprite* pTile : m_tiles)
				if (pTile->HitTest(newDest))
					newDest = m_dest;	// no change of destination in case of a collision with a tile
			m_dest = newDest;

			// Set new velocity and new animation only if new direction different than current direction (dot product test)
			if (Dot(m_player.GetVelocity(), newDir) < 0.1)
			{
				m_player.SetVelocity(100 * newDir);
				m_player.SetAnimation(newAnim);
			}

			// a little bit of trickery to ensure the player is always alogned to the tiles
			m_player.SetPosition(64 * floorf(m_player.GetPosition().m_x / 64) + 32, 64 * floorf(m_player.GetPosition().m_y / 64) + 32);
		}
		m_player.Update(t);

		// Guards control
		if (m_guards[0]->GetPosition().m_x < 64 * 1 + 32)
		{
			m_guards[0]->SetAnimation("walkR");
			m_guards[0]->SetVelocity(CVector(SPEED_GUARD_0, 0));
		}
		if (m_guards[0]->GetPosition().m_x > 64 * 18 + 32)
		{
			m_guards[0]->SetAnimation("walkL");
			m_guards[0]->SetVelocity(CVector(-SPEED_GUARD_0, 0));
		}

		if (m_guards[1]->GetPosition().m_y > 64 * 12 + 32)
		{
			m_guards[1]->SetAnimation("walkR");
			m_guards[1]->SetVelocity(CVector(0, -SPEED_GUARD_1));
		}
		if (m_guards[1]->GetPosition().m_y < 64 * 7 + 32)
		{
			m_guards[1]->SetAnimation("walkL");
			m_guards[1]->SetVelocity(CVector(0, SPEED_GUARD_1));
		}

		if (m_guards[2]->GetPosition().m_x < 64 * 13 + 32)
		{
			m_guards[2]->SetAnimation("walkR");
			m_guards[2]->SetVelocity(CVector(SPEED_GUARD_2, 0));
		}
		if (m_guards[2]->GetPosition().m_x > 64 * 23 + 32)
		{
			m_guards[2]->SetAnimation("walkL");
			m_guards[2]->SetVelocity(CVector(-SPEED_GUARD_2, 0));
		}
	}


	for (CSprite* pGuard : m_guards)
		pGuard->Update(GetTime());

	if (badChoice)
	{
		guardSpeechSceen.SetAnimation("speech_lose");

		playerSpeechSceen.SetAnimation("speech_lose");
	}
	else if (goodChoice)
	{
		playerSpeechSceen.SetAnimation("speech_pass");

		guardSpeechSceen.SetAnimation("speech_pass");
	}
	else
	{
		guardSpeechSceen.SetAnimation("speech_idle");
		guardSpeechSceen.SetPosition(speechSceneStandSecurity.GetX(), speechSceneStandSecurity.GetY() + 150);

		playerSpeechSceen.SetAnimation("speech_idle");
		playerSpeechSceen.SetPosition(speechSceneStandPlayer.GetX(), speechSceneStandPlayer.GetY() + 180);
	}


	if (sparkRotation >= 0 && sparkRotation <= 360)
	{
		sparkRotation++;
	}
	else
	{
		sparkRotation = 0;
	}

	spark1.SetRotation(sparkRotation);
	spark2.SetRotation(sparkRotation);
	spark3.SetRotation(sparkRotation);

	//speech scene sprites
	speechSceneBG.Update(t);
	speechSceneStandPlayer.Update(t);
	speechSceneStandSecurity.Update(t);

	guardSpeechSceen.Update(t);
	playerSpeechSceen.Update(t);

	speechSceneEnter1.Update(t);
	speechSceneEnter2.Update(t);
	speechSceneEnter3.Update(t);
	speechSceneEnter4.Update(t);
	startSpeechSceneBG.Update(t);

	spark1.Update(t);
	spark2.Update(t);
	spark3.Update(t);

	prisonerHint.Update(t);
	pillowHint.Update(t);
	onionHint.Update(t);
	
	cursor.Update(t);

	textbox1.Update(t);
	textbox2.Update(t);
	textbox3.Update(t);


	// LINE OF SIGHT TEST
	for (CSprite* pGuard : m_guards)
	{
		// by default, we assume each guard can become a killer
		m_pKiller = pGuard;

		// browse through all tiles - if line of sight test shows any tile to obscure the player, then we have no killer after all
		for (CSprite* pTile : m_tiles)
		{
			// Check intersection of the "Guard - Player" sight line with both diagonals of the tile.
			// If there is intersection - there is no killer - so, m_pKiller = NULL;

			// TO DO:
			// Call the Intersection function twice, once for each diagonal of the tile
			// If the function returns true in any case, call the following:

			if (Intersection(pGuard->GetPosition(), m_player.GetPosition(), CVector(pTile->GetLeft(), pTile->GetTop()), CVector(pTile->GetRight(), pTile->GetBottom())))
				m_pKiller = NULL;

			if (Intersection(pGuard->GetPosition(), m_player.GetPosition(), CVector(pTile->GetLeft(), pTile->GetBottom()), CVector(pTile->GetRight(), pTile->GetTop())))
				m_pKiller = NULL;


			if (m_pKiller == NULL)
				break;	// small optimisation, if line of sight test already failed, no point to look further
		}

		// if the player is in plain sight of the guard...
		if (m_pKiller)
		{
			// Additional test - only killing if the player within 60 degrees from the guard's front (they have no eyes in the back of their head)
			CVector v = m_player.GetPosition() - pGuard->GetPosition();

			// TO DO: Calculate the Dot Product of the displacement vector (v - calculated above) and the guard's velocity vector.
			// Normalise both vectors for the dot!
			// If the result is greater than 0.5, the player is within 60 degrees from the front of the guard.
			// Otherwise, the guard should not see the player (again, m_pKiller = NULL)

			if (Dot(Normalise(v), Normalise(pGuard->GetVelocity() - (v / 2))) <= 0.5)
				m_pKiller = NULL;
		
		}
		
		// if still the killer found - the game is over and look no more!
		if (m_pKiller && !securityPass)
		{
			speechScene = true;
			return;
		}
	}
}

void CMyGame::OnDraw(CGraphics* g)
{
	m_tiles.for_each(&CSprite::Draw, g);

	if (!unlocked)
	{
		background.Draw(g);
	}
	else
	{
		background1.Draw(g);
	}

	for (CSprite* pGuard : m_guards)
		pGuard->Draw(g);

	Ebutton.Draw(g);
	Ebutton1.Draw(g);

	m_player.Draw(g);

	paper1.Draw(g);
	paper2.Draw(g);

	spark1.Draw(g);
	spark2.Draw(g);
	spark3.Draw(g);

	doorway.Draw(g);
	doorway1.Draw(g);

	table1.Draw(g);
	table2.Draw(g);

	darkScreen.Draw(g);
	darkScreen1.Draw(g);

	riddleMenu.Draw(g);
	backButton.Draw(g);
	backButton1.Draw(g);

	if (riddleMenuEntered)
	{
		*g << font(28) << color(CColor::Black()) << xy(riddleMenu.GetX() - 230, riddleMenu.GetY() - 35) << "Q1";
		*g << font(28) << color(CColor::Black()) << xy(riddleMenu.GetX() - 20, riddleMenu.GetY() - 35) << "Q2";
		*g << font(28) << color(CColor::Black()) << xy(riddleMenu.GetX() + 190, riddleMenu.GetY() - 35) << "Q3";

		if (question1)
		{
			*g << font(35) << color(CColor::Black()) << xy(riddleMenu.GetX() - 275, riddleMenu.GetY() + 120) << "Q1: Who can finish a book without";
			*g << font(35) << color(CColor::Black()) << xy(riddleMenu.GetX() - 230, riddleMenu.GetY() + 80) << "finishing a sentence ? ";
		}
		else if (question2)
		{
			*g << font(35) << color(CColor::Black()) << xy(riddleMenu.GetX() - 275, riddleMenu.GetY() + 120) << "Q2: What loses its head and gets it";
			*g << font(35) << color(CColor::Black()) << xy(riddleMenu.GetX() - 225, riddleMenu.GetY() + 80) << "back at night?";
		}
		else if (question3)
		{
			*g << font(35) << color(CColor::Black()) << xy(riddleMenu.GetX() - 275, riddleMenu.GetY() + 120) << "Q3: Take off my skin"<< " - "<<"I won" << "'" << "t cry,";
			*g << font(35) << color(CColor::Black()) << xy(riddleMenu.GetX() - 225, riddleMenu.GetY() + 80) << "but you will! What am I?";
		}

		if (unlocked)
		{
			*g << font(50) << color(CColor::Black()) << xy(riddleMenu.GetX() - 175, riddleMenu.GetY() + 120) << "CORRECT!";
			*g << font(50) << color(CColor::Black()) << xy(riddleMenu.GetX() - 175, riddleMenu.GetY() + 60) << "Door unlocked";
		}
	}

	prisonerHint.Draw(g);
	prisonerHint1.Draw(g);
	prisoner.Draw(g);

	pillowHint.Draw(g);
	pillowHint1.Draw(g);
	pillow.Draw(g);

	onionHint.Draw(g);
	onionHint1.Draw(g);
	onion.Draw(g);

	//speech scene sprites
	speechSceneBG.Draw(g);
	speechSceneStandPlayer.Draw(g);
	speechSceneStandSecurity.Draw(g);

	guardSpeechSceen.Draw(g);
	playerSpeechSceen.Draw(g);

	textmenubox.Draw(g);
	textbox1.Draw(g);
	textbox2.Draw(g);
	textbox3.Draw(g);

	playerChat.Draw(g);
	securityChat.Draw(g);


	///////////////////  Speech Text 1  ////////////////////////////////////
	
	if (whichSecurity == 1 && chatCD == 0)
	{
		//security speech
		if (text1)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 150, securityChat.GetY() + 10) << "Hey stop right there!";
		}
		if (text3 && !badChoice && !goodChoice && !okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "What do mean how can you help me!";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "You shouldn" << "'" << "t be here after school hours";
		}

		if (goodChoice && text4 && !badChoice && !okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "Hmm Mr Makoto huh";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "if he said it" << "'" << "s okay go on ahead";
		}

		if (badChoice && text4bad && !goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 230, securityChat.GetY() + 10) << "ALRIGHT! You" << "'" << "re coming with me kid";
		}

		if (okChoice && !badChoice && !goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "That" << "'" << "s not good enough,";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "tell me why you are really here!";
		}

		if (okChoice && !badChoice && goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "That sounds okay to me,";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "just don" << "'" << "t hang around too long!";
		}

		if (badChoice && text5bad && !goodChoice && okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 10) << "*Blushes* HUH WHAT GET OUT OF HERE NOW!";
		}


		//player speech

		if (text2)
		{
			*g << font(28) << color(CColor::Black()) << xy(playerChat.GetX() - 150, playerChat.GetY() + 10) << "How can I help you sir?";
		}

		if (text3 && !badChoice && !goodChoice && !okChoice)
		{
			if (!highlighted1)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY()) << "Hey man watch your tone with me";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() + 6) << "Hey man watch your tone with me";
			}

			if (!highlighted2)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 20) << "Mr Makoto gave me permission to grab some";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 5) << "books from the library and also get my";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 30) << "homework from my desk, that" << "'" << "s all sir";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 26) << "Mr Makoto gave me permission to grab some";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 1) << "books from the library and also get my";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 24) << "homework from my desk, that" << "'" << "s all sir";
			}

			if (!highlighted3)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox3.GetX() - 265, textbox3.GetY() + 10) << "Can" << "'" << "t a guy just hang out, after school hours";
				*g << font(28) << color(CColor::Black()) << xy(textbox3.GetX() - 265, textbox3.GetY() - 15) << "in the place that" << "'" << "ll prepare me for my future";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox3.GetX() - 265, textbox3.GetY() + 16) << "Can" << "'" << "t a guy just hang out, after school hours";
				*g << font(28) << color(CColor::Black()) << xy(textbox3.GetX() - 265, textbox3.GetY() - 9) << "in the place that" << "'" << "ll prepare me for my future";
			}
		}

		if (okChoice && !badChoice && !goodChoice)
		{
			if (!highlighted1)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY()) << "I came here to talk to you handsome";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() + 6) << "I came here to talk to you handsome";
			}

			if (!highlighted2)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 20) << "I got permission to grab";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 5) << "my homework that" << "'" << "s all my man";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 30) << "*this better work or I" << "'" << "m screwed*";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 26) << "I got permission to grab";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 1) << "my homework that" << "'" << "s all my man";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 24) << "*this better work or I" << "'" << "m screwed*";
			}
		}

	}

	/////////////////  Speech Text 2  /////////////////////////////////////

	if (whichSecurity == 2 && chatCD == 0)
	{
		//security speech
		if (text3 && !badChoice && !goodChoice && !okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "Hey you there";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "what do you think you are doing";
		}

		if (goodChoice && text4 && !badChoice && !okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 30) << "The librarian hmm that should be fine,";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 5) << "but kid did the librarian mention me...";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 20) << "*cough* I mean just make it quick";

		}

		if (badChoice && text4bad && !goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "THE WHAT? YOUR MAKING NO SENSE";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "I" << "'" << "M KICKING YOU OUT!";
		}

		if (okChoice && !badChoice && !goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 30) << "That" << "'" << "s disgusting but";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 5) << "I still can" << "'" << "t let you hang around";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 20) << "you got any other reason?";
		}

		if (okChoice && !badChoice && goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "If Mr Makoto said it" << "'" << "s okay";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "go on ahead then";
		}

		if (badChoice && text5bad && !goodChoice && okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "WHAT! YOUR OUT OF HERE AND";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "YOUR LUCKY IM JUST KICKING YOU OUT";
		}


		//player speech

		if (text3 && !badChoice && !goodChoice && !okChoice)
		{
			if (!highlighted1)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY()) << "I was looking for the one piece?";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() + 6) << "I was looking for the one piece?";
			}

			if (!highlighted2)
			{
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 20) << "I had an overdue Library book I was returning";
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 5) << "the librarian said its okay for me to come back";
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 30) << "after school hours, you understand right?";
			}
			else
			{
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 26) << "I had an overdue Library book I was returning";
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 1) << "the librarian said its okay for me to come back";
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 24) << "after school hours, you understand right?";
			}

			if (!highlighted3)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox3.GetX() - 265, textbox3.GetY() + 10) << "I" << "'" << "m going to be honest I left my sandwich";
				*g << font(28) << color(CColor::Black()) << xy(textbox3.GetX() - 265, textbox3.GetY() - 15) << "in my desk drawer and I" << "'" << "m really hungry";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox3.GetX() - 265, textbox3.GetY() + 16) << "I" << "'" << "m going to be honest I left my sandwich";
				*g << font(28) << color(CColor::Black()) << xy(textbox3.GetX() - 265, textbox3.GetY() - 9) << "in my desk drawer and I" << "'" << "m really hungry";
			}
		}

		if (okChoice && !badChoice && !goodChoice)
		{
			if (!highlighted1)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() + 20) << "Come on my man It" << "'" << "s a Friday";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() - 5) << "by the time I can get the sandwich";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() - 30) << "my drawer will stink as bad as you";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() + 26) << "Come on my man It" << "'" << "s a Friday";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() +1) << "by the time I can get the sandwich";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 210, textbox1.GetY() - 24) << "my drawer will stink as bad as you";
			}

			if (!highlighted2)
			{
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 270, textbox2.GetY() + 20) << "Umm yes, my homework was under the sandwich";
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 290, textbox2.GetY() - 5) << "thought I" << "'" << "d grab both and Mr Makoto said it was cool";
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 270, textbox2.GetY() - 30) << "*PLEASE WORK IT" << "'" << "S ALL I CAN COME UP WITH*";
			}
			else
			{
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 270, textbox2.GetY() + 26) << "Umm yes, my homework was under the sandwich";
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 290, textbox2.GetY() + 1) << "thought I" << "'" << "d grab both and Mr Makoto said it was cool";
				*g << font(25) << color(CColor::Black()) << xy(textbox2.GetX() - 270, textbox2.GetY() - 24) << "*PLEASE WORK IT" << "'" << "S ALL I CAN COME UP WITH*";
			}
		}

	}

	/////////////////////  Speech Text 3  ////////////////////////////////////////

	if (whichSecurity == 3 && chatCD == 0)
	{
		//security speech
		if (text3 && !badChoice && !goodChoice && !okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 10) << "Hey what are you doing here!";
		}

		if (goodChoice && text4 && !badChoice && !okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "I never heard of this,";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "but it sounds right just make it quick";

		}

		if (badChoice && text4bad && !goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 10) << "I" << "'" << "M NOT MARRIED KID, NOW GET OUT OF HERE!";
		}

		if (okChoice && !badChoice && !goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "It" << "'" << "s 9pm you can" << "'" << "t just be";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "hanging out here by yourself";
		}

		if (okChoice && !badChoice && goodChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 20) << "I guess that sounds fine";
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() - 5) << "just hurry up and grab your stuff";
		}

		if (badChoice && text5bad && !goodChoice && okChoice)
		{
			*g << font(28) << color(CColor::Black()) << xy(securityChat.GetX() - 240, securityChat.GetY() + 10) << "I" << "'" << "m going to escort you out now";
		}


		//player speech

		if (text3 && !badChoice && !goodChoice && !okChoice)
		{
			if (!highlighted1)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() + 20) << "I was looking for you! your wife called";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() - 5) << "she said, " << "‘" << "she wants to get a divorce" << "’";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() - 30) << "you should go fix that";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() + 26) << "I was looking for you! your wife called";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() + 1) << "she said, "<<"‘"<<"she wants to get a divorce" << "’";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() - 24) << "you should go fix that";
			}

			if (!highlighted2)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 20) << "You didn" << "'" << "t know I was called by Mr Makoto";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 5) << "to clean the classroom and";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 30) << "library after school hours?";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 26) << "You didn" << "'" << "t know I was called by Mr Makoto";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() + 1) << "to clean the classroom and";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 265, textbox2.GetY() - 24) << "library after school hours?";
			}

			if (!highlighted3)
			{
				*g << font(25) << color(CColor::Black()) << xy(textbox3.GetX() - 272, textbox3.GetY() + 20) << "OHHHHH, I was just hanging out WOW look at the time";
				*g << font(25) << color(CColor::Black()) << xy(textbox3.GetX() - 272, textbox3.GetY() - 5) << "school is over, time flies when you have fun right.";
				*g << font(25) << color(CColor::Black()) << xy(textbox3.GetX() - 272, textbox3.GetY() - 30) << "I" << "'" << "ll just grab my stuff";
			}
			else
			{
				*g << font(25) << color(CColor::Black()) << xy(textbox3.GetX() - 272, textbox3.GetY() + 26) << "OHHHHH, I was just hanging out WOW look at the time";
				*g << font(25) << color(CColor::Black()) << xy(textbox3.GetX() - 272, textbox3.GetY() + 1) << "school is over, time flies when you have fun right.";
				*g << font(25) << color(CColor::Black()) << xy(textbox3.GetX() - 272, textbox3.GetY() - 24) << "I" << "'" << "ll just grab my stuff";
			}
		}

		if (okChoice && !badChoice && !goodChoice)
		{
			if (!highlighted1)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() + 10) << "I was daydreaming about ripping up";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() - 15) << "my homework and here we are";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() + 16) << "I was daydreaming about ripping up";
				*g << font(28) << color(CColor::Black()) << xy(textbox1.GetX() - 240, textbox1.GetY() - 9) << "my homework and here we are";
			}

			if (!highlighted2)
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 270, textbox2.GetY() + 10) << "Alright Alright I" << "'" << "m sorry I fell asleep";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 270, textbox2.GetY() - 15) << "and completely lost track of time";
			}
			else
			{
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 270, textbox2.GetY() + 16) << "Alright Alright I" << "'" << "m sorry I fell asleep";
				*g << font(28) << color(CColor::Black()) << xy(textbox2.GetX() - 270, textbox2.GetY() - 9) << "and completely lost track of time";
			}
		}

	}

	speechSceneEnter1.Draw(g);
	speechSceneEnter2.Draw(g);
	speechSceneEnter3.Draw(g);
	speechSceneEnter4.Draw(g);

	static int z = 0; 
	if ((z++ / 10) % 2 == 1 && startSpeechScene)
	{
		startSpeechSceneBG.Draw(g);
	}

	if (IsGameOverMode())
	{
		if (gamewon)
		{
			*g << font(100) << color(CColor::Green()) << vcenter << center << " GAME WON ";
		}
		else
		{
			*g << font(100) << color(CColor::Red()) << vcenter << center << " GAME OVER ";
		}
	}

	cursor.Draw(g);
}

void CMyGame::SpeechText1()
{
	if (textStart == 0 && text1cd == -1)
	{
		text1 = true;
		text1cd = 150;
	}
	if (text1cd == 0 && text2cd == -1)
	{
		text1 = false;

		text2 = true;
		text2cd = 150;
	}
	if (text2cd == 0 && text3cd == -1)
	{
		text2 = false;

		text3 = true;
	}
	if (goodChoice && text3 && text4cd == -1)
	{
		text3 = false;
		text4cd = 150;
		text4 = true;
	}
	if (goodChoice && text4 && text4cd == 0 && speechSceneExitCD == -1 && !badChoice && !speechSceneExit)
	{
		text3 = false;

		speechSceneExit = true;

		speechSceneExitCD = 20;
	}

	if (speechSceneExitCD > 0)
	{
		speechSceneExitCD--;
	}

	if (badChoice && !goodChoice)
	{
		text3 = false;

		if (okChoice)
		{
			text5bad = true;
		}
		else
		{
			text4bad = true;
		}


		GameOver();
	}

	//////////////////////////////////////////////////
	if (!badChoice && !goodChoice)
	{
		if (text3 && cursor.HitTest(&textbox1) && clicked)
		{
			badChoice = true;
		}
		if (text3 && cursor.HitTest(&textbox2) && clicked)
		{
			goodChoice = true;
		}
		if (text3 && cursor.HitTest(&textbox3) && clicked && !okChoice)
		{
			okChoice = true;
		}
	}
	/////////////////////////////
	if (textStart > 0)
	{
		textStart--;
	}

	if (text1cd > 0)
	{
		text1cd--;
	}

	if (text2cd > 0)
	{
		text2cd--;
	}

	if (text3cd > 0)
	{
		text3cd--;
	}

	if (text4cd > 0)
	{
		text4cd--;
	}

	if (text5cd > 0)
	{
		text5cd--;
	}
	/////////////////////////////
	if (!badChoice && !goodChoice && !speechSceneExit)
	{
		if (cursor.HitTest(&textbox1))
		{
			highlighted1 = true;

			textbox1.SetAnimation("textbox1.2");
		}
		else
		{
			highlighted1 = false;

			textbox1.SetAnimation("textbox1.1");
		}

		if (cursor.HitTest(&textbox2))
		{
			highlighted2 = true;

			textbox2.SetAnimation("textbox2.2");
		}
		else
		{
			highlighted2 = false;

			textbox2.SetAnimation("textbox2.1");
		}

		if (cursor.HitTest(&textbox3))
		{
			highlighted3 = true;

			textbox3.SetAnimation("textbox3.2");
		}
		else
		{
			highlighted3 = false;

			textbox3.SetAnimation("textbox3.1");
		}
	}

	if (speechScene)
	{
		if (text2)
		{
			playerChat.SetImage("playerChat");
			playerChat.SetPosition(playerSpeechSceen.GetX() + 320, playerSpeechSceen.GetY() + 90);
		}
		else
		{
			playerChat.SetPosition(-1000, -110);
		}


		// security speech
		if (text1 || text3 || text4 ||text5)
		{
			securityChat.SetImage("securityChat");
			securityChat.SetPosition(guardSpeechSceen.GetX() - 320, guardSpeechSceen.GetY() + 85);
		}
		
		if (text2)
		{
			securityChat.SetPosition(-1000, -110);
		}
	}
}

void CMyGame::SpeechText2()
{
	if (textStart == 0 && text3cd == -1)
	{
		text3 = true;
	}
	if (goodChoice && text3 && text4cd == -1)
	{
		text3 = false;
		text4cd = 150;
		text4 = true;
	}
	if (goodChoice && text4 && text4cd == 0 && speechSceneExitCD == -1 && !badChoice && !speechSceneExit)
	{
		text3 = false;

		speechSceneExit = true;

		speechSceneExitCD = 70;
	}

	if (speechSceneExitCD > 0)
	{
		speechSceneExitCD--;
	}

	//////////////////////////////////////////////////
	if (!badChoice && !goodChoice)
	{
		if (text3 && cursor.HitTest(&textbox1) && clicked)
		{
			badChoice = true;
		}
		if (text3 && cursor.HitTest(&textbox2) && clicked)
		{
			goodChoice = true;
		}
		if (text3 && cursor.HitTest(&textbox3) && clicked && !okChoice)
		{
			okChoice = true;
		}
	}
	///////////////////////////////////////////////////
	if (badChoice && !goodChoice)
	{
		text3 = false;

		if (okChoice)
		{
			text5bad = true;
		}
		else
		{
			text4bad = true;
		}


		GameOver();
	}

	/////////////////////////////
	if (textStart > 0)
	{
		textStart--;
	}

	if (text1cd > 0)
	{
		text1cd--;
	}

	if (text2cd > 0)
	{
		text2cd--;
	}

	if (text3cd > 0)
	{
		text3cd--;
	}

	if (text4cd > 0)
	{
		text4cd--;
	}

	if (text5cd > 0)
	{
		text5cd--;
	}
	/////////////////////////////
	if (!badChoice && !goodChoice && !speechSceneExit)
	{
		if (cursor.HitTest(&textbox1))
		{
			highlighted1 = true;

			textbox1.SetAnimation("textbox1.2");
		}
		else
		{
			highlighted1 = false;

			textbox1.SetAnimation("textbox1.1");
		}

		if (cursor.HitTest(&textbox2))
		{
			highlighted2 = true;

			textbox2.SetAnimation("textbox2.2");
		}
		else
		{
			highlighted2 = false;

			textbox2.SetAnimation("textbox2.1");
		}

		if (cursor.HitTest(&textbox3))
		{
			highlighted3 = true;

			textbox3.SetAnimation("textbox3.2");
		}
		else
		{
			highlighted3 = false;

			textbox3.SetAnimation("textbox3.1");
		}
	}

	if (speechScene)
	{
		// security speech
		if (text1 || text3 || text4 || text5)
		{
			securityChat.SetImage("securityChat");
			securityChat.SetPosition(guardSpeechSceen.GetX() - 320, guardSpeechSceen.GetY() + 85);
		}

		if (text2)
		{
			securityChat.SetPosition(-1000, -110);
		}
	}
}

void CMyGame::SpeechText3()
{
	if (textStart == 0 && text3cd == -1)
	{
		text3 = true;
	}
	if (goodChoice && text3 && text4cd == -1)
	{
		text3 = false;
		text4cd = 150;
		text4 = true;
	}
	if (goodChoice && text4 && text4cd == 0 && speechSceneExitCD == -1 && !badChoice && !speechSceneExit)
	{
		text3 = false;

		speechSceneExit = true;

		speechSceneExitCD = 70;
	}

	if (speechSceneExitCD > 0)
	{
		speechSceneExitCD--;
	}

	//////////////////////////////////////////////////
	if (!badChoice && !goodChoice)
	{
		if (text3 && cursor.HitTest(&textbox1) && clicked)
		{
			badChoice = true;
		}
		if (text3 && cursor.HitTest(&textbox2) && clicked)
		{
			goodChoice = true;
		}
		if (text3 && cursor.HitTest(&textbox3) && clicked && !okChoice)
		{
			okChoice = true;
		}
	}
	///////////////////////////////////////////////////
	if (badChoice && !goodChoice)
	{
		text3 = false;

		if (okChoice)
		{
			text5bad = true;
		}
		else
		{
			text4bad = true;
		}


		GameOver();
	}

	/////////////////////////////
	if (textStart > 0)
	{
		textStart--;
	}

	if (text1cd > 0)
	{
		text1cd--;
	}

	if (text2cd > 0)
	{
		text2cd--;
	}

	if (text3cd > 0)
	{
		text3cd--;
	}

	if (text4cd > 0)
	{
		text4cd--;
	}

	if (text5cd > 0)
	{
		text5cd--;
	}
	/////////////////////////////
	if (!badChoice && !goodChoice && !speechSceneExit)
	{
		if (cursor.HitTest(&textbox1))
		{
			highlighted1 = true;

			textbox1.SetAnimation("textbox1.2");
		}
		else
		{
			highlighted1 = false;

			textbox1.SetAnimation("textbox1.1");
		}

		if (cursor.HitTest(&textbox2))
		{
			highlighted2 = true;

			textbox2.SetAnimation("textbox2.2");
		}
		else
		{
			highlighted2 = false;

			textbox2.SetAnimation("textbox2.1");
		}

		if (cursor.HitTest(&textbox3))
		{
			highlighted3 = true;

			textbox3.SetAnimation("textbox3.2");
		}
		else
		{
			highlighted3 = false;

			textbox3.SetAnimation("textbox3.1");
		}
	}

	if (speechScene)
	{
		// security speech
		if (text1 || text3 || text4 || text5)
		{
			securityChat.SetImage("securityChat");
			securityChat.SetPosition(guardSpeechSceen.GetX() - 320, guardSpeechSceen.GetY() + 85);
		}

		if (text2)
		{
			securityChat.SetPosition(-1000, -110);
		}
	}
}

void CMyGame::SpeachSceneEnter()
{
	if (speechScene)
	{
		if (!enter && !startSpeechScene && startspeechsceneCD != 0)
		{
			m_player.SetVelocity(0, 0);

			m_guards[0]->SetVelocity(CVector(0, 0));
			m_guards[0]->SetAnimation("idle");

			m_guards[1]->SetVelocity(CVector(0, 0));
			m_guards[1]->SetAnimation("idle");

			m_guards[2]->SetVelocity(CVector(0, 0));
			m_guards[2]->SetAnimation("idle");

			startSpeechScene = true;

			enter = true;
		}

		if (speechSceneEnter)
		{
			speechSceneBG.SetPosition(796.5, 447);
			speechSceneBG.SetImage("speechSceneBG");

			speechSceneEnter1.SetPosition(796.5, 670.5);
			speechSceneEnter1.SetImage("speechSceneEnter1");

			speechSceneEnter2.SetPosition(796.5, 223.5);
			speechSceneEnter2.SetImage("speechSceneEnter2");

			speechSceneEnter3.SetPosition(-275, 447);
			speechSceneEnter3.SetImage("speechSceneEnter3");

			speechSceneEnter4.SetPosition(1868, 447);
			speechSceneEnter4.SetImage("speechSceneEnter4");

			speechSceneStandSecurity.SetImage("speechSceneStandSecurity");
			speechSceneStandSecurity.SetPosition(-153, 530);

			speechSceneStandPlayer.SetImage("speechSceneStandPlayer");
			speechSceneStandPlayer.SetPosition(1760, 340);

			textmenubox.SetImage("textmenubox");
			textmenubox.SetPosition(796.5, 200);

			textbox1.SetAnimation("textbox1.1");
			textbox1.SetPosition(textmenubox.GetX() - 325, textmenubox.GetY() + 65);

			textbox2.SetAnimation("textbox2.1");
			textbox2.SetPosition(textmenubox.GetX() + 325, textmenubox.GetY() + 65);

			textbox3.SetAnimation("textbox3.1");
			textbox3.SetPosition(textmenubox.GetX(), textmenubox.GetY() - 60);

			speechTextEnter = true;
		}

		if (speechSceneEnter && speechsceneCD > 0)
		{
			speechsceneCD--;
		}

		if (speechSceneCharEnter && speechsceneCharCD > 0)
		{
			speechsceneCharCD--;
		}

		if (speechSceneEnter && speechsceneCD == 0)
		{
			speechSceneEnter = false;

			speechsceneCD = 50;

			speechSceneEnter1.SetVelocity(0, 100);

			speechSceneEnter2.SetVelocity(0, -100);
		}

		if (speechSceneEnter1.HitTest(796.5, 1292))
		{
			speechSceneEnter1.SetVelocity(0, 0);
		}

		if (speechSceneEnter2.HitTest(796.5, -398))
		{
			speechSceneEnter2.SetVelocity(0, 0);
		}

		if (speechSceneCharEnter && speechsceneCharCD == 0)
		{
			speechsceneCharCD = 300;

			chatCD = 120;

			speechSceneCharEnter = false;

			speechSceneStandSecurity.SetVelocity(400, 0);

			speechSceneStandPlayer.SetVelocity(-400, 0);
		}

		if (speechSceneStandPlayer.HitTest(165, 340))
		{
			speechSceneStandPlayer.SetVelocity(0, 0);
		}

		if (speechSceneStandSecurity.HitTest(1440, 530))
		{
			speechSceneStandSecurity.SetVelocity(0, 0);
		}

		if (startSpeechScene && startspeechsceneCD != 0)
		{
			startspeechsceneCD--;

			startSpeechSceneBG.SetPosition(796.5, 447);
			startSpeechSceneBG.SetImage("startSpeechSceneBG");
		}

		if (!speechSceneEnter && startSpeechScene && startspeechsceneCD == 0)
		{
			startSpeechScene = false;
			startspeechsceneCD = 100;
			startSpeechSceneBG.SetPosition(-6640, 384);

			speechSceneEnter = true;

			speechSceneCharEnter = true;
		}
	}
	else
	{
		speechSceneBG.SetPosition(-6640, 384);

		speechSceneEnter1.SetPosition(-6640, 576);

		speechSceneEnter2.SetPosition(-6640, 192);

		speechSceneEnter3.SetPosition(-6020, 384);

		speechSceneEnter4.SetPosition(-6300, 384);

		speechSceneStandSecurity.SetPosition(-6053, 500);

		speechSceneStandPlayer.SetPosition(-6450, 200);

		textmenubox.SetPosition(-1450, 200);

		textbox1.SetPosition(-1000,-100);

		textbox2.SetPosition(-1200, -100);

		textbox3.SetPosition(-1400, -100);

		securityChat.SetPosition(-1000, -110);
	}
}

void CMyGame::SpeachSceneExit()
{
	if (speechScene && speechSceneExit && speechSceneExitCD == 0)
	{
		speechScene = false;

		speechSceneExit = false;

		speechSceneExitCD = -1;

		m_guards[0]->SetVelocity(CVector(SPEED_GUARD_0, 0));
		m_guards[0]->SetAnimation("walkR");

		m_guards[1]->SetVelocity(CVector(0, SPEED_GUARD_1));
		m_guards[1]->SetAnimation("walkL");

		m_guards[2]->SetVelocity(CVector(-SPEED_GUARD_2, 0));
		m_guards[2]->SetAnimation("walkL");

		enter = false;

		chatCD = -1;

		text1cd = -1;
		text2cd = -1;
		text3cd = -1;
		text4cd = -1;
		text5cd = -1;

		text1 = false;;
		text2 = false;
		text3 = false;
		text4bad = false;
		text4 = false;
		text5 = false;
		text5bad = false;

		goodChoice = false;
		badChoice = false;
		okChoice = false;

		securityPass = true;
		securityMemoryLossCD = 300;
	}

	if (securityPass && securityMemoryLossCD > 0)
	{
		securityMemoryLossCD--;
	}
	if (securityPass && securityMemoryLossCD == 0)
	{
		securityPass = false;
	}
}

void CMyGame::CollectHints()
{
	if (m_player.HitTest(&paper1triggerbox) && IsKeyDown(SDLK_e))
	{
		spark1.SetPosition(-1000, -1000);
		paper1.SetPosition(-1000, -1000);
		paper1triggerbox.SetPosition(-1000, -1000);

		prisonerHint1.SetImage("prisonerHint1");
		prisonerHint1.SetPosition(796.5, 447);

		darkScreen.SetImage("darkScreen");
		darkScreen.SetPosition(796.5, 447);

		pressedEcounter = 90;

		prisonerCollected = true;
	}

	if (pressedEcounter == 0)
	{
		prisonerHint1.SetPosition(-1000, -1000);

		darkScreen.SetPosition(-1000, -1000);
	}




	if (m_player.HitTest(&paper2triggerbox) && IsKeyDown(SDLK_e))
	{
		spark2.SetPosition(-1000, -1000);
		paper2.SetPosition(-1000, -1000);
		paper2triggerbox.SetPosition(-1000, -1000);

		pillowHint1.SetImage("pillowHint1");
		pillowHint1.SetPosition(796.5, 447);

		darkScreen.SetImage("darkScreen");
		darkScreen.SetPosition(796.5, 447);

		pressedEcounter = 90;

		pillowCollected = true;
	}

	if (pressedEcounter == 0)
	{
		pillowHint1.SetPosition(-1000, -1000);

		darkScreen.SetPosition(-1000, -1000);
	}




	if (m_player.HitTest(&paper3triggerbox) && IsKeyDown(SDLK_e))
	{
		spark3.SetPosition(-1000, -1000);
		paper3triggerbox.SetPosition(-1000, -1000);

		onionHint1.SetImage("onionHint1");
		onionHint1.SetPosition(796.5, 447);

		darkScreen.SetImage("darkScreen");
		darkScreen.SetPosition(796.5, 447);

		pressedEcounter = 90;

		onionCollected = true;
	}

	if (pressedEcounter == 0)
	{
		onionHint1.SetPosition(-1000, -1000);

		darkScreen.SetPosition(-1000, -1000);
	}


	if (pressedEcounter > 0)
	{
		pressedEcounter--;
	}


	if (m_player.HitTest(&paper1triggerbox))
	{
		Ebutton.SetImage("Ebutton");
		Ebutton.SetPosition(710, 670);
	}
	else if (m_player.HitTest(&paper2triggerbox))
	{
		Ebutton.SetImage("Ebutton");
		Ebutton.SetPosition(900, 320);
	}
	else if (m_player.HitTest(&paper3triggerbox))
	{
		Ebutton.SetImage("Ebutton");
		Ebutton.SetPosition(1505, 770);
	}
	else
	{
		Ebutton.SetPosition(-1485, 760);
	}


}

void CMyGame::OpenRiddleDoor()
{
	if (!draggingPrisoner || !draggingPillow || !draggingOnion)
	{
		if (prisonerCollected && riddleMenuEntered && !prisonerAttached)
		{
			prisonerHint.SetImage("prisonerHint");
			prisonerHint.SetPosition(1435, 700);
		}
		if (pillowCollected && riddleMenuEntered && !pillowAttached)
		{
			pillowHint.SetImage("pillowHint");
			pillowHint.SetPosition(1435, 450);
		}
		if (onionCollected && riddleMenuEntered && !onionAttached)
		{
			onionHint.SetImage("onionHint");
			onionHint.SetPosition(1435, 200);
		}
	}
	if (draggingPrisoner || draggingPillow || draggingOnion)
	{
		cursor.SetAnimation("cursor3");
	}

	if (m_player.HitTest(&triggerBox))
	{
		Ebutton1.SetImage("Ebutton");
		Ebutton1.SetPosition(935, 840);
	}
	else
	{
		Ebutton1.SetPosition(-1485, 760);
	}

	if (m_player.HitTest(&triggerBox) && IsKeyDown(SDLK_e) && !riddleMenuEntered)
	{
		riddleMenu.SetImage("riddleMenu");
		riddleMenu.SetPosition(796.5, 447);

		darkScreen1.SetImage("darkScreen");
		darkScreen1.SetPosition(796.5, 447);

		backButton.SetImage("backButton");
		backButton.SetPosition(1100, 240);

		riddleBox1.SetPosition(578.5, 325);
		riddleBox2.SetPosition(778.5, 325);
		riddleBox3.SetPosition(978.5, 325);

		riddleMenuEntered = true;

		question1 = true;
	}

	if (cursor.HitTest(&backButton))
	{
		backButton1.SetImage("backButton1");
		backButton1.SetPosition(1100, 240);
	}
	else
	{
		backButton1.SetPosition(-1100, -1240);
	}

	if (cursor.HitTest(&backButton1) && clicked && riddleMenuEntered && (!draggingPrisoner || !draggingPillow || !draggingOnion))
	{
		riddleMenu.SetPosition(-1796.5, -447);

		darkScreen1.SetPosition(-1796.5, -447);

		backButton.SetPosition(-1100, -1240);

		prisonerHint.SetPosition(-1435, -1700);
		prisoner.SetPosition(-1578.5, 325);

		pillowHint.SetPosition(-1435, -1450);
		pillow.SetPosition(-1435, -1450);

		onionHint.SetPosition(-1435, -1200);
		onion.SetPosition(-1435, -1200);


		riddleBox1.SetPosition(-1578.5, 325);
		riddleBox2.SetPosition(-1578.5, 325);
		riddleBox3.SetPosition(-1578.5, 325);

		riddleMenuEntered = false;
	}



	if (cursor.HitTest(&prisonerHint) && clicked)
	{
		draggingPrisoner = true;
	}
	if (draggingPrisoner && !prisonerAttached)
	{
		prisonerHint.SetPosition(cursor.GetX(), cursor.GetY() - 90);
	}
	if (cursor.HitTest(&prisonerHint) && !clicked)
	{
		draggingPrisoner = false;
	}
	////////
	if (cursor.HitTest(&pillowHint) && clicked)
	{
		draggingPillow = true;

	}
	if (draggingPillow && !pillowAttached)
	{
		pillowHint.SetPosition(cursor.GetX(), cursor.GetY() - 90);
	}
	if (cursor.HitTest(&pillowHint) && !clicked)
	{
		draggingPillow = false;
	}
	////////
	if (cursor.HitTest(&onionHint) && clicked)
	{
		draggingOnion = true;
	}
	if (draggingOnion && !onionAttached)
	{
		onionHint.SetPosition(cursor.GetX(), cursor.GetY() - 90);
	}
	if (cursor.HitTest(&onionHint) && !clicked)
	{
		draggingOnion = false;
	}



	if (riddleMenuEntered)
	{
		if (question1 && !question2 && !question3)
		{
			if (prisonerHint.HitTest(578.5, 325))
			{
				prisonerAttached = true;

				question1 = false;
				question2 = true;
			}
			if (prisonerAttached)
			{
				prisonerHint.SetPosition(-1500, -1020);

				prisoner.SetImage("prisoner");
				prisoner.SetPosition(578.5, 325);
			}
		}



		if (!question1 && question2 && !question3)
		{
			if (pillowHint.HitTest(778.5, 325))
			{
				pillowAttached = true;

				question2 = false;
				question3 = true;
			}
			if (pillowAttached)
			{
				pillowHint.SetPosition(-1500, -1020);

				pillow.SetImage("pillow");
				pillow.SetPosition(790, 325);
			}
		}



		if (!question1 && !question2 && question3)
		{
			if (onionHint.HitTest(978.5, 325))
			{
				onionAttached = true;

				question3 = false;
				unlocked = true;
			}
			if (onionAttached)
			{
				onionHint.SetPosition(-1500, -1020);

				onion.SetImage("onion");
				onion.SetPosition(1000, 325);
			}
		}
	}

	if (unlocked && m_player.HitTest(&finish) && !riddleMenuEntered)
	{
		backButton1.SetPosition(-1000, -100);

		gamewon = true;
		GameOver();
	}
}

/////////////////////////////////////////////////////
// Game Life Cycle

// one time initialisation
void CMyGame::OnInitialize()
{
	// Create Tiles
	for (int y = 0; y < 15; y++)
		for (int x = 0; x < 35; x++)
		{
			if (m_tileLayout[y][x] == ' ')
				continue;

			int nTile = 5;
			if (y > 0 && m_tileLayout[y - 1][x] == ' ') nTile -= 3;
			if (y < 11 && m_tileLayout[y + 1][x] == ' ') nTile += 3;
			if (x > 0 && m_tileLayout[y][x - 1] == ' ') nTile--;
			if (x < 20 && m_tileLayout[y][x + 1] == ' ') nTile++;
			if (nTile == 5 && x > 0 && y > 0 && m_tileLayout[y - 1][x - 1] == ' ') nTile = 14;
			if (nTile == 5 && x < 20 && y > 0 && m_tileLayout[y - 1][x + 1] == ' ') nTile = 13;
			if (nTile == 5 && x > 0 && y < 11 && m_tileLayout[y + 1][x - 1] == ' ') nTile = 11;
			if (nTile == 5 && x < 20 && y < 11 && m_tileLayout[y + 1][x + 1] == ' ') nTile = 10;
			
			nTile--;
			m_tiles.push_back(new CSprite(x * 64.f + 32.f, y * 64.f + 32.f, new CGraphics("tiles.png", 3, 5, nTile % 3, nTile / 3), 0));
		}


	background.LoadImageW("background.png", "background");
	background1.LoadImageW("background1.png", "background1");

	doorway.LoadImageW("doorway.png", "doorway");
	doorway1.LoadImageW("doorway.png", "doorway1");

	table1.LoadImageW("table.png", "table");
	table2.LoadImageW("table.png", "table");

	spark1.LoadAnimation("spark.png", "spark", CSprite::Sheet(8, 1).Row(0).From(0).To(7));
	spark2.LoadAnimation("spark.png", "spark", CSprite::Sheet(8, 1).Row(0).From(0).To(7));
	spark3.LoadAnimation("spark.png", "spark", CSprite::Sheet(8, 1).Row(0).From(0).To(7));

	paper1.LoadImageW("paper.png", "paper");
	paper2.LoadImageW("paper.png", "paper");

	paper1triggerbox.LoadImageW("TriggerBox.png", "paperTriggerBox");
	paper2triggerbox.LoadImageW("TriggerBox.png", "paperTriggerBox");
	paper3triggerbox.LoadImageW("TriggerBox.png", "paperTriggerBox");

	triggerBox.LoadImageW("TriggerBox.png", "TriggerBox");

	finish.LoadImageW("TriggerBox.png", "finish");

	riddleBox1.LoadImageW("TriggerBox.png", "riddleBox1");
	riddleBox2.LoadImageW("TriggerBox.png", "riddleBox2");
	riddleBox3.LoadImageW("TriggerBox.png", "riddleBox3");

	Ebutton.LoadImageW("Ebutton.png", "Ebutton");
	Ebutton1.LoadImageW("Ebutton.png", "Ebutton");

	prisonerHint.LoadImageW("prisonerHint.png", "prisonerHint");
	prisonerHint1.LoadImageW("prisonerHintBig.png", "prisonerHint1");
	prisoner.LoadImageW("prisoner.png", "prisoner");

	pillowHint.LoadImageW("pillowHint.png", "pillowHint");
	pillowHint1.LoadImageW("pillowHintBig.png", "pillowHint1");
	pillow.LoadImageW("pillow.png", "pillow");

	onionHint.LoadImageW("onionHint.png", "onionHint");
	onionHint1.LoadImageW("onionHintBig.png", "onionHint1");
	onion.LoadImageW("onion.png", "onion");

	cursor.LoadAnimation("cursor.png", "cursor1", CSprite::Sheet(3, 1).Row(0).From(0).To(0));
	cursor.LoadAnimation("cursor.png", "cursor2", CSprite::Sheet(3, 1).Row(0).From(1).To(1));
	cursor.LoadAnimation("cursor.png", "cursor3", CSprite::Sheet(3, 1).Row(0).From(2).To(2));

	darkScreen.LoadImageW("darkScreen.png", "darkScreen");
	darkScreen1.LoadImageW("darkScreen.png", "darkScreen");

	riddleMenu.LoadImageW("riddleMenu.png", "riddleMenu");
	backButton.LoadImageW("backButton.png", "backButton");
	backButton1.LoadImageW("backButton1.png", "backButton1");

	line1.LoadImageW("line v.png", "line");
	line11.LoadImageW("line h.png", "line");
	line12.LoadImageW("line h.png", "line");

	line2.LoadImageW("line h.png", "line");

	line3.LoadImageW("line h.png", "line");

	// Prepare the Player for the first use
	m_player.LoadAnimation("runR.png", "walkR", CSprite::Sheet(8, 1).Row(0).From(0).To(7));
	m_player.LoadAnimation("runD.png", "walkD", CSprite::Sheet(8, 1).Row(0).From(0).To(7));
	m_player.LoadAnimation("runL.png", "walkL", CSprite::Sheet(8, 1).Row(0).From(0).To(7));
	m_player.LoadAnimation("runU.png", "walkU", CSprite::Sheet(8, 1).Row(0).From(0).To(7));
	m_player.LoadAnimation("idle.png", "idleL", CSprite::Sheet(3, 1).Row(0).From(0).To(0));
	m_player.LoadAnimation("idle.png", "idleR", CSprite::Sheet(3, 1).Row(0).From(1).To(1));
	m_player.LoadAnimation("idle.png", "idleU", CSprite::Sheet(3, 1).Row(0).From(2).To(2));

	// Prepare the enemies for the first use
	for (int i = 0; i < 3; i++)
	{
		CSprite *pGuard = new CSprite(0, 0, "guard.png", 0);
		pGuard->LoadAnimation("guard.png", "walkR", CSprite::Sheet(6, 6).Row(4).From(0).To(5));
		pGuard->LoadAnimation("guard.png", "walkL", CSprite::Sheet(6, 6).Row(0).From(0).To(5));
		pGuard->LoadAnimation("guard.png", "idle", CSprite::Sheet(6, 6).Row(5).From(0).To(0));
		m_guards.push_back(pGuard);
	}

	//speach scene sprites
	guardSpeechSceen.LoadAnimation("securityReactions.png", "speech_idle", CSprite::Sheet(10, 1).Row(0).From(0).To(0));
	guardSpeechSceen.LoadAnimation("securityReactions.png", "speech_thinking", CSprite::Sheet(10, 1).Row(0).From(4).To(8));
	guardSpeechSceen.LoadAnimation("securityReactions.png", "speech_pass", CSprite::Sheet(10, 1).Row(0).From(3).To(3));
	guardSpeechSceen.LoadAnimation("securityReactions.png", "speech_lose", CSprite::Sheet(10, 1).Row(0).From(9).To(9));

	playerSpeechSceen.LoadAnimation("reactions.png", "speech_idle", CSprite::Sheet(7, 1).Row(0).From(3).To(3));
	playerSpeechSceen.LoadAnimation("reactions.png", "speech_thinking1", CSprite::Sheet(7, 1).Row(0).From(0).To(0));
	playerSpeechSceen.LoadAnimation("reactions.png", "speech_thinking2", CSprite::Sheet(7, 1).Row(0).From(4).To(4));
	playerSpeechSceen.LoadAnimation("reactions.png", "speech_positive", CSprite::Sheet(7, 1).Row(0).From(5).To(5));
	playerSpeechSceen.LoadAnimation("reactions.png", "speech_negative", CSprite::Sheet(7, 1).Row(0).From(1).To(1));
	playerSpeechSceen.LoadAnimation("reactions.png", "speech_pass", CSprite::Sheet(7, 1).Row(0).From(2).To(2));
	playerSpeechSceen.LoadAnimation("reactions.png", "speech_lose", CSprite::Sheet(7, 1).Row(0).From(7).To(7));

	startSpeechSceneBG.LoadImageW("start speech enter scene.png", "startSpeechSceneBG");
	speechSceneBG.LoadImageW("speech scene background.png","speechSceneBG");
	speechSceneStandPlayer.LoadImageW("speech scene stand.png", "speechSceneStandPlayer");
	speechSceneStandSecurity.LoadImageW("speech scene stand.png", "speechSceneStandSecurity");
	speechSceneEnter1.LoadImageW("speech scene enter h.png", "speechSceneEnter1");
	speechSceneEnter2.LoadImageW("speech scene enter h.png", "speechSceneEnter2");
	speechSceneEnter3.LoadImageW("speech scene enter v.png", "speechSceneEnter3");
	speechSceneEnter4.LoadImageW("speech scene enter v.png", "speechSceneEnter4");

	textmenubox.LoadImageW("textmenubox.png", "textmenubox");

	textbox1.LoadAnimation("textbox.png", "textbox1.1", CSprite::Sheet(2, 1).Row(0).From(0).To(0));
	textbox1.LoadAnimation("textbox.png", "textbox1.2", CSprite::Sheet(2, 1).Row(0).From(1).To(1));

	textbox2.LoadAnimation("textbox.png", "textbox2.1", CSprite::Sheet(2, 1).Row(0).From(0).To(0));
	textbox2.LoadAnimation("textbox.png", "textbox2.2", CSprite::Sheet(2, 1).Row(0).From(1).To(1));

	textbox3.LoadAnimation("textbox.png", "textbox3.1", CSprite::Sheet(2, 1).Row(0).From(0).To(0));
	textbox3.LoadAnimation("textbox.png", "textbox3.2", CSprite::Sheet(2, 1).Row(0).From(1).To(1));

	playerChat.LoadImageW("playerChat.png", "playerChat");
	securityChat.LoadImageW("securityChat.png", "securityChat");

}

// called when a new game is requested (e.g. when F2 pressed)
// use this function to prepare a menu or a welcome screen
void CMyGame::OnDisplayMenu()
{
	StartGame();	// exits the menu mode and starts the game mode
}

// called when a new game is started
// as a second phase after a menu or a welcome screen
void CMyGame::OnStartGame()
{
	line1.SetImage("line");
	line1.SetPosition(1400, 130);
	line11.SetImage("line");
	line11.SetPosition(1250, 250);
	line12.SetImage("line");
	line12.SetPosition(100, 250);

	line2.SetImage("line");
	line2.SetPosition(1250, 450);

	line3.SetImage("line");
	line3.SetPosition(100, 450);

	cursor.SetY(-100);
	cursor.SetAnimation("cursor1");

	background.SetImage("background");
	background.SetPosition(798, 445.5);

	finish.SetImage("finish");
	finish.SetPosition(865, 950);

	background1.SetImage("background1");
	background1.SetPosition(800, 444);

	doorway.SetImage("doorway");
	doorway.SetPosition(1254, 397);

	doorway1.SetImage("doorway1");
	doorway1.SetPosition(102, 397);

	table1.SetImage("table");
	table1.SetPosition(305, 492);

	table2.SetImage("table");
	table2.SetPosition(603, 492);

	spark1.SetAnimation("spark");
	spark1.SetPosition(660, 660);

	spark2.SetAnimation("spark");
	spark2.SetPosition(855, 305);

	spark3.SetAnimation("spark");
	spark3.SetPosition(1475, 760);

	paper1.SetImage("paper");
	paper1.SetPosition(660, 658);
	paper1triggerbox.SetImage("paperTriggerBox");
	paper1triggerbox.SetPosition(660, 658);

	paper2.SetImage("paper");
	paper2.SetPosition(855, 303);
	paper2triggerbox.SetImage("paperTriggerBox");
	paper2triggerbox.SetPosition(855, 303);

	paper3triggerbox.SetImage("paperTriggerBox");
	paper3triggerbox.SetPosition(1475, 760);

	triggerBox.SetImage("TriggerBox");
	triggerBox.SetPosition(865, 830);

	// Reinitialise the player
	m_player.SetPosition(64 * 23 + 32, 64 + 32);
	m_player.SetVelocity(0, 0);
	m_player.SetAnimation("idle");
	m_dest = m_player.GetPosition();

	// Reinitialise the guards
	m_guards[0]->SetPosition(64 * 17 + 32, 64 * 1 + 32); //2
	m_guards[0]->SetAnimation("walkL");
	m_guards[0]->SetVelocity(CVector(-SPEED_GUARD_0, 0));

	m_guards[1]->SetPosition(64 * 7 + 32, 64 * 7 + 32);
	m_guards[1]->SetAnimation("walkL");
	m_guards[1]->SetVelocity(CVector(0, SPEED_GUARD_1));

	m_guards[2]->SetPosition(64 * 15 + 32, 64 * 10 + 32);
	m_guards[2]->SetAnimation("walkR");
	m_guards[2]->SetVelocity(CVector(SPEED_GUARD_2, 0));

	m_pKiller = NULL;

	HideMouse();
}

// called when a new level started - first call for nLevel = 1
void CMyGame::OnStartLevel(Sint16 nLevel)
{
}

// called when the game is over
void CMyGame::OnGameOver()
{
}

// one time termination code
void CMyGame::OnTerminate()
{
}

/////////////////////////////////////////////////////
// Keyboard Event Handlers

void CMyGame::OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	if (sym == SDLK_F4 && (mod & (KMOD_LALT | KMOD_RALT)))
		StopGame();
	if (sym == SDLK_SPACE)
		PauseGame();
	if (sym == SDLK_F2)
		NewGame();


}

void CMyGame::OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode)
{
}


/////////////////////////////////////////////////////
// Mouse Events Handlers

void CMyGame::OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle)
{
	if (IsGameMode()) cursor.SetPosition(x, y);
}

void CMyGame::OnLButtonDown(Uint16 x, Uint16 y)
{
	clicked = true;
	cursor.SetAnimation("cursor2");
}

void CMyGame::OnLButtonUp(Uint16 x,Uint16 y)
{
	draggingPrisoner = false;
	draggingPillow = false;
	draggingOnion = false;

	clicked = false;
	cursor.SetAnimation("cursor1");
}

void CMyGame::OnRButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonUp(Uint16 x,Uint16 y)
{
}

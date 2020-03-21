#include "GSPlay.h"
#include "Shaders.h"
#include "Texture.h"
#include "Models.h"
#include "Camera.h"
#include "Font.h"
#include "Sprite2D.h"
#include "Sprite3D.h"
#include "Text.h"
#include "SpriteAnimation.h"
#include <ctime>
#include <string>

extern int screenWidth; //need get on Graphic engine
extern int screenHeight; //need get on Graphic engine
extern int clickstat;
int board[H][W];
int landed[H][W];
int block[4][4];
int x, y, lastRow, score;
bool success;
int nextBlockType = rand() % 7; 
int block_list[7][4][4] =
{
	{
		{ 0, 1, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 1, 0, 0 }
	},
	{
		{ 0, 0, 1, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	},
	{
		{ 0, 0, 1, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 0, 0 }
	},
	{
		{ 0, 1, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 0 }
	},
	{
		{ 0, 1, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 0, 0 }
	},
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	},
	{
		{ 0, 1, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 0, 0 }
	}
};
bool makeBlock();
bool moveBlock(int x2, int y2);
bool moveToBottom();
bool rotateBlock();
bool isCollide(int x2, int y2);
bool isFilled(int x);
void clearLine(int x);

GSPlay::GSPlay()
{
	score = 0;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			board[i][j] = 0;
			landed[i][j] = 0;
		}
	}
	makeBlock();
}


GSPlay::~GSPlay()
{

}


void GSPlay::Init()
{
	auto model = ResourceManagers::GetInstance()->GetModel("Sprite2D");
	auto texture = ResourceManagers::GetInstance()->GetTexture("dot");
	auto shader = ResourceManagers::GetInstance()->GetShader("TextureShader");
	for (int row = 0; row < H; row++) {
		for (int col = 0; col < W; col++) {
			m_board[row][col] = std::make_shared<Sprite2D>(model, shader, texture);
			m_board[row][col]->Set2DPosition(30.4 + col*32.8, 61.2 + row*32.8);
			if (board[row][col] == 1 && row > 0) {				
				m_board[row][col]->SetSize(32.8, 32.8);
			}
			else m_board[row][col]->SetSize(0, 0);
		}
	}

	//BackGround
	texture = ResourceManagers::GetInstance()->GetTexture("gp_background");
	m_BackGround = std::make_shared<Sprite2D>(model, shader, texture);
	m_BackGround->Set2DPosition(screenWidth / 2, screenHeight / 2);
	m_BackGround->SetSize(screenWidth, screenHeight);

	//pause button
	texture = ResourceManagers::GetInstance()->GetTexture("pause_sbtn");
	std::shared_ptr<GameButton> button = std::make_shared<GameButton>(model, shader, texture);
	button->Set2DPosition(380, 290);
	button->SetSize(50, 50);
	button->SetOnClick([]() {
		GameStateMachine::GetInstance()->ChangeState(StateTypes::STATE_PauseMenu);
	});
	m_listButton.push_back(button);

	//setting button
	texture = ResourceManagers::GetInstance()->GetTexture("setting_sbtn");
	button = std::make_shared<GameButton>(model, shader, texture);
	button->Set2DPosition(445, 290);
	button->SetSize(50, 50);
	button->SetOnClick([]() {
		GameStateMachine::GetInstance()->ChangeState(StateTypes::STATE_Setting);
	});
	m_listButton.push_back(button);

	//main menu button
	texture = ResourceManagers::GetInstance()->GetTexture("main_menu_btn");
	button = std::make_shared<GameButton>(model, shader, texture);
	button->Set2DPosition(412, 340);
	button->SetSize(120, 30);
	button->SetOnClick([]() {
		GameStateMachine::GetInstance()->PopState();
	});
	m_listButton.push_back(button);

	//Next block
	switch (nextBlockType) {
	case 0:
		texture = ResourceManagers::GetInstance()->GetTexture("i");
		break;
	case 1:
		texture = ResourceManagers::GetInstance()->GetTexture("j");
		break;
	case 2:
		texture = ResourceManagers::GetInstance()->GetTexture("z");
		break;
	case 3:
		texture = ResourceManagers::GetInstance()->GetTexture("s");
		break;
	case 4:
		texture = ResourceManagers::GetInstance()->GetTexture("t");
		break;
	case 5:
		texture = ResourceManagers::GetInstance()->GetTexture("o");
		break;
	case 6:
		texture = ResourceManagers::GetInstance()->GetTexture("l");
		break;
	}
	
	m_nextBlock = std::make_shared<Sprite2D>(model, shader, texture);
	m_nextBlock->Set2DPosition(415, 200);
	m_nextBlock->SetSize(60, 60);


	//text game score
	shader = ResourceManagers::GetInstance()->GetShader("TextShader");
	std::shared_ptr<Font> font = ResourceManagers::GetInstance()->GetFont("telelower");
	m_score = std::make_shared< Text>(shader, font, std::to_string(score), TEXT_COLOR::GREEN, 0.7);
	m_score->Set2DPosition(Vector2(425, 98));
}

void GSPlay::Exit()
{

}


void GSPlay::Pause()
{

}

void GSPlay::Resume()
{

}


void GSPlay::HandleEvents()
{

}

void GSPlay::HandleKeyEvents(int key, bool bIsPressed)
{
	if (!clickstat) {
		//do something
		switch (key) {
		case 32:
			success = moveToBottom();
			if (success) {
				for (int i = 0; i < H; i++) {
					for (int j = 0; j < W; j++) {
						landed[i][j] = board[i][j];
					}
				}
				for (int row = lastRow - 3; row <= lastRow; row++) {
					if (isFilled(row)) {
						clearLine(row);
						score++;
					}
				}
				success = makeBlock();
			}
			break;
		case 40:
			if (isCollide(x, y + 1)) {
				for (int i = 0; i < H; i++) {
					for (int j = 0; j < W; j++) {
						landed[i][j] = board[i][j];
					}
				}
				for (int row = lastRow - 3; row <= lastRow; row++) {
					if (isFilled(row)) {
						clearLine(row);
						score++;
					}
				}
				success = makeBlock();
			}
			else success = moveBlock(x, y + 1);
			break;
		case 39:
			success = moveBlock(x + 1, y);
			break;
		case 38:
			success = rotateBlock();
			break;
		case 37:
			success = moveBlock(x - 1, y);
			break;
		default:	success = false;
		}
		clickstat = 1;
	}
	else {
		clickstat = 0;
	}
}

void GSPlay::HandleTouchEvents(int x, int y, bool bIsPressed)
{
	if (!clickstat) {
		//do something
		for (auto it : m_listButton)
		{
			(it)->HandleTouchEvents(x, y, bIsPressed);
			if ((it)->IsHandle()) break;
		}
		clickstat = 1;
	}
	else {
		clickstat = 0;
	}
}

void GSPlay::Update(float deltaTime)
{
	for (auto obj : m_listSpriteAnimations)
	{
		obj->Update(deltaTime);
	}
	m_BackGround->Update(deltaTime);
	for (auto it : m_listButton)
	{
		it->Update(deltaTime);
	}
	auto model = ResourceManagers::GetInstance()->GetModel("Sprite2D");
	auto shader = ResourceManagers::GetInstance()->GetShader("TextureShader");
	auto texture = ResourceManagers::GetInstance()->GetTexture("i");
	switch (nextBlockType) {
	case 0:
		texture = ResourceManagers::GetInstance()->GetTexture("i");
		break;
	case 1:
		texture = ResourceManagers::GetInstance()->GetTexture("j");
		break;
	case 2:
		texture = ResourceManagers::GetInstance()->GetTexture("z");
		break;
	case 3:
		texture = ResourceManagers::GetInstance()->GetTexture("s");
		break;
	case 4:
		texture = ResourceManagers::GetInstance()->GetTexture("t");
		break;
	case 5:
		texture = ResourceManagers::GetInstance()->GetTexture("o");
		break;
	case 6:
		texture = ResourceManagers::GetInstance()->GetTexture("l");
		break;
	}

	m_nextBlock = std::make_shared<Sprite2D>(model, shader, texture);
	m_nextBlock->Set2DPosition(415, 200);
	m_nextBlock->SetSize(60, 60);
	if (success) {
		texture = ResourceManagers::GetInstance()->GetTexture("dot");
		for (int row = 0; row < H; row++) {
			for (int col = 0; col < W; col++) {
				m_board[row][col] = std::make_shared<Sprite2D>(model, shader, texture);
				m_board[row][col]->Set2DPosition(30.4 + col*32.8, 61.2 + row*32.8);
				if (board[row][col] == 1 && row > 0) {
					m_board[row][col]->SetSize(32.8, 32.8);
				}
				else m_board[row][col]->SetSize(0, 0);
			}
		}
	}
	shader = ResourceManagers::GetInstance()->GetShader("TextShader");
	std::shared_ptr<Font> font = ResourceManagers::GetInstance()->GetFont("telelower");
	m_score = std::make_shared<Text>(shader, font, std::to_string(score), TEXT_COLOR::GREEN, 0.7);
	m_score->Set2DPosition(Vector2(425, 98));
}

void GSPlay::Draw()
{
	m_BackGround->Draw();
	m_nextBlock->Draw();
	for (auto i = 0; i < H; i++) {
		for (auto j = 0; j < W; j++) {
			m_board[i][j]->Draw();
		}
	}
	for (auto obj : m_listSpriteAnimations)
	{
		obj->Draw();
	}
	for (auto it : m_listButton)
	{
		it->Draw();
	}
	m_score->Draw();
	
}

void GSPlay::SetNewPostionForBullet()
{
}


bool makeBlock() {
	x = 4;
	y = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			block[i][j] = block_list[nextBlockType][i][j];
			board[i][j + 4] = block[i][j];
		}
	}
	nextBlockType = rand() % 7;
	success = true;
	return success;
}

bool moveBlock(int x2, int y2) {
	if (!isCollide(x2, y2)) {
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				board[y + i][x + j] -= block[i][j];
			}
		}
		x = x2;
		y = y2;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				board[y + i][x + j] += block[i][j];
			}
		}
		success = true;
	}
	else success = false;
	lastRow = y + 3;
	return success;
}

bool moveToBottom() {
	int y2 = y;
	while (!isCollide(x, y2)) y2++;
	return moveBlock(x, y2-1);
}

bool rotateBlock() {
	int tmp[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			tmp[i][j] = block[i][j];
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			block[i][j] = tmp[3 - j][i];
		}
	}
	if (isCollide(x, y)) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				block[i][j] = tmp[i][j];
			}
		}
		return false;
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			board[y + i][x + j] -= tmp[i][j];
			board[y + i][x + j] += block[i][j];
		}
	}
	return true;
}

bool isCollide(int x2, int y2) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[i][j] == 1 && (y2 + i >= H || x2 + j < 0 || x2 + j >= W)) return true;
			if (block[i][j] + landed[y2 + i][x2 + j] > 1) return true;
		}
	}
	return false;
}

bool isFilled(int x) {
	bool isFilled = true;
	for (int i = 0; i < W; i++) {
		if (landed[x][i] == 0) isFilled = false;
	}
	return isFilled;
}

void clearLine(int x) {
	for (int i = x; i > 0; i--) {
		for (int j = 0; j < W; j++) {
			landed[i][j] = landed[i - 1][j];
			board[i][j] = board[i - 1][j];
		}
	}
}
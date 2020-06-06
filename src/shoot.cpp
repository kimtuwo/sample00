#include "DxLib.h"
#include<stdlib.h>
#include<time.h>

// 画像データサイズ
struct CharPicData {
	int width, height;
	char image[128];
};

// Windowのエリア
struct Erea {
	int min_x;
	int min_y;
	int max_x;
	int max_y;
};

// キャラの基本形
class Character {
public:
	Character(int _x, int _y, Erea &erea, CharPicData &data)
		: x(_x), y(_y), mova_erea(erea), cdata(data)
	{
		image_handle = LoadGraph(cdata.image);
	}
	~Character(){}

	int get_x(void) const { return x; }
	int get_y(void) const { return y; }
	int get_width(void) const { return cdata.width; }
	int get_height(void) const { return cdata.height; }
	void draw(void) { DrawGraph(x, y, image_handle, FALSE); }

	void move(int ix, int iy) {
		x += ix;
		if(x < mova_erea.min_x) {x = mova_erea.min_x; }
		else if(x > mova_erea.max_x) { x = mova_erea.max_x; }

		y += iy;
		if(y < mova_erea.min_y) {x = mova_erea.min_y; }
		else if(y > mova_erea.max_y) { x = mova_erea.max_y; }
	}

protected:
	int x, y;
	int image_handle;

	Erea mova_erea;
	CharPicData cdata;
};

// 敵キャラ
class Enemy : public Character {
public:
	Enemy(int _x, int _y, Erea &erea, CharPicData &data): 
		Character(_x, _y, erea, data), 
		dx(0), init_x(_x) {}
	~Enemy(){}

	void init_motion(int _dx) { dx = _dx; }
	void update(void) {
		x += dx;
		if(x < mova_erea.min_x) {
			x = mova_erea.min_x;
			dx = -dx;
		}
		else if(x > mova_erea.max_x) {
			 x = mova_erea.max_x; 
			dx = -dx;
		}
	}
	void reset(void) { x=init_x; }

private:
	int dx;
	int init_x;
};

// 弾丸、Fire
class Shot : public Character {
public:
	Shot(int _x, int _y, Erea &erea, CharPicData &data, int _dy): 
		Character(_x, _y, erea, data), dy(_dy) {}
	~Shot(){}

	bool update(void) {
		y += dy;
		return (y > mova_erea.min_y) && (y < mova_erea.max_y);
	}

private:
	int dy;
};

//---------------------------------------------------------------------
// Basic data
#define WINDOW_WIDTH   640
#define WINDOW_HEIGHT  480

// Window erea
Erea window_erea = {0, 0, WINDOW_WIDTH-32, WINDOW_HEIGHT-32};

// Character data
//                      width, height, image
CharPicData PlayerPic = {22,    31,    "player000.png"};
CharPicData EnemyPic  = {22,    29,    "enemy000.png"};
CharPicData ShotPic   = {16,    16,    "star.png"};
CharPicData EshotPic  = {16,    16,    "fire.png"};

#define INI_PX   309  // Player 初期 x座標
#define INI_PY   400  // Player 初期 y座標
#define INI_EX0    0  // Enemy0 初期 x座標
#define INI_EY0   16  // Enemy0 初期 y座標
#define INI_EX1  640  // Enemy1 初期 x座標
#define INI_EY1  100  // Enemy1 初期 y座標

#define MAX_SHOTS      5  // 同時に発射可能な弾丸数
#define SHOT_INTERVAL  3  // Shotの発射間隔
#define SHOT_SPEED    12  // Shotのスピード

//---------------------------------------------------------------------
bool checkHit(Character &a, Character &b)  // あたり判定(a の方が b より大きい前提)
{
	if(a.get_x() <= b.get_x()+b.get_width() && a.get_x()+a.get_width() >= b.get_x() &&
	   a.get_y() <= b.get_y()+b.get_height() && a.get_y()+a.get_height() >= b.get_y()) {
		   return true;
	}

	return false;
}

//---------------------------------------------------------------------
bool checkFire(Character &a, Character &b, int ratio_n, int ratio_d)  // 発射判定
{
	return (a.get_x() + 14 >= b.get_x() + 11 && a.get_x() + 8 <= b.get_x() + 11 && rand() % ratio_d < ratio_n );
}

//---------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	ChangeWindowMode(TRUE);
	DxLib_Init();
	SetDrawScreen(DX_SCREEN_BACK);

	DrawString( 240, 208, "shooting game", GetColor(255,255,255));
	DrawString( 240, 230, "please press any key", GetColor(255,255,255));
	ScreenFlip();
	WaitKey();

	int score = 0;

	Character player(INI_PX,  INI_PY,  window_erea, PlayerPic);
	Enemy enemy0(INI_EX0, INI_EY0, window_erea, EnemyPic);
	Enemy enemy1(INI_EX1, INI_EY1, window_erea, EnemyPic);

	enemy0.init_motion(8);
	enemy1.init_motion(-10);

	Shot *eshot[2] = {0, 0}; // 敵のfire
	Shot *p_shot[MAX_SHOTS]; // 自機の弾丸
	
	for(int i=0 ; i < MAX_SHOTS ; i++){ p_shot[i] = 0; }

	int shot_count = 0;
	int shot_index = 0;
	int shot_interval = 0;

	while (!ProcessMessage()) {
		ClearDrawScreen();

		// 自機の位置更新
		if (CheckHitKey(KEY_INPUT_LEFT)) {
			player.move(-8, 0);
		}
		if (CheckHitKey(KEY_INPUT_RIGHT)) {
			player.move(8, 0);
		}

		// 弾丸発射
		if (CheckHitKey(KEY_INPUT_SPACE)) {
			if(shot_count < MAX_SHOTS && shot_interval == 0) {
				p_shot[shot_index % MAX_SHOTS] = new Shot(player.get_x() + 8, INI_PY, window_erea, ShotPic, -SHOT_SPEED);
				shot_index++;
				shot_count++;
				shot_interval = SHOT_INTERVAL;
			}
		}

		if (CheckHitKey(KEY_INPUT_ESCAPE)) {
			break;
		}

		// 敵の位置 更新
		enemy0.update();
		enemy1.update();

		// 弾丸 更新
		for(int n=0 ; n < MAX_SHOTS ; n++) {
			if(p_shot[n]) {
				if(p_shot[n]->update()==false) {
					delete p_shot[n];
					p_shot[n] = 0;
					shot_count--;
				} else {
					p_shot[n]->draw();
				}
			}
		}
		if(shot_interval > 0) shot_interval--;

		// Fire 更新
		for(int n=0 ; n < 2 ; n++) {
			if(eshot[n]) {
				if(eshot[n]->update()==false) {
					delete eshot[n];
					eshot[n] = 0;
				} else {
					eshot[n]->draw();
				}
			}
		}

		// あたり判定（敵）
		for(int n=0 ; n < MAX_SHOTS ; n++) {
			if(p_shot[n]) {
				if(checkHit(enemy0, *p_shot[n])) {
					score += 10;
					enemy0.reset();
				}
				if(checkHit(enemy1, *p_shot[n])) {
					score += 10;
					enemy1.reset();
				}
			}
		}
		DrawFormatString(0, 0, GetColor(255, 255, 255), "SCORE : %d", score);

		// あたり判定（自機）
		if ((eshot[0] && checkHit(player, *eshot[0])) || (eshot[1] && checkHit(player, *eshot[1]))){
			DrawString( 240, 208, "Game Over!", GetColor(255,255,255));
			ScreenFlip();
			WaitTimer(2000);
			break;
		}

		// Fire発射
		srand((unsigned int)time(NULL));
		if(eshot[0] == 0 && checkFire(player, enemy0, 8, 5)){
			eshot[0] = new Shot(enemy0.get_x() + 8, INI_EY0, window_erea, EshotPic, SHOT_SPEED);
		}
		if(eshot[1] == 0 && checkFire(player, enemy1, 8, 3)){
			eshot[1] = new Shot(enemy1.get_x() + 8, INI_EY1, window_erea, EshotPic, SHOT_SPEED-6);
		}

		if(score>=1000){
			DrawString( 240, 208, "Game Clear!", GetColor(255,255,255));
			ScreenFlip();
			WaitTimer(2000);
			break;
		}

		player.draw();
		enemy0.draw();
		enemy1.draw();

		ScreenFlip();
	}

	DxLib_End();
	return 0;
}

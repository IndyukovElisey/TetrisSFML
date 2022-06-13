#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <conio.h>
#include <thread>
#include <random>
#include <cstdlib>
#include <stdlib.h>
#include <mutex>
#include <iomanip>
#include <vector>
using namespace std;

enum rot { clockwise, anticlockwise };
enum action { UP, LEFT, DOWN, RIGHT, ROT_CLOCKWISE, ROT_ANTICLOCKWISE };

int queue[5];
int score;
bool if_gameover;
bool if_exit = false;
int prev_color = 0;

int figures_count = 0;
chrono::milliseconds delay;
const chrono::milliseconds min_delay = 250ms;

const int WIDTH = 10, HEIGHT = 15;

sf::RenderWindow window(sf::VideoMode(220, 520), "Tetris");
sf::Image field_image;
sf::Texture field_texture;
sf::Sprite field_sprite;
sf::Font font;
sf::Text text_next("", font, 30);
sf::Text text_score("", font, 30);
bool if_not_first_menu = 0;
bool isMenu = 0;

	class figure
	{
	protected:
		int size;
		bool** cells;
		int color;
	public:
		figure(int s) :size(s)
		{
			cells = new bool* [size];

			for (int i = 0; i < size; i++)
				cells[i] = new bool[size];

			for (int i = 0; i < size; i++)
				for (int j = 0; j < size; j++)
					cells[i][j] = false;
		}
		figure(const figure & f)
		{
			size = f.size;
			cells = new bool* [size];

			for (int i = 0; i < size; i++)
				cells[i] = new bool[size];

			for (int i = 0; i < size; i++)
				for (int j = 0; j < size; j++)
					cells[i][j] = f.cells[i][j];
		}
		virtual ~figure()
		{
			for (int i = 0; i < size; i++)
				delete[] cells[i];
			delete[] cells;
		}
		figure& operator=(const figure & fig)
		{
			if (this != &fig)
			{
				for (int i = 0; i < size; i++)
					delete[] cells[i];
				delete[] cells;

				size = fig.size;
				cells = new bool* [size];

				for (int i = 0; i < size; i++)
					cells[i] = new bool[size];

				for (int i = 0; i < size; i++)
					for (int j = 0; j < size; j++)
						cells[i][j] = fig.cells[i][j];
			}
			return *this;
		}
		void RotateFigure(rot r)
		{
			//if(size==4)

			bool** temp = new bool* [size];

			for (int i = 0; i < size; i++)
				temp[i] = new bool[size];

			for (int i = 0; i < size; i++)
			{
				for (int j = size - 1; j >= 0; j--)
				{
					if (r == clockwise)
						temp[i][size - 1 - j] = cells[j][i];
					else
						temp[j][i] = cells[i][size - 1 - j];
				}
			}

			for (int i = 0; i < size; i++)
				for (int j = 0; j < size; j++)
					cells[i][j] = temp[i][j];
			for (int i = 0; i < size; i++)
				delete[] temp[i];
			delete[] temp;
		}
		const int GetSize() const
		{
			return size;
		}
		bool* operator[](int n)
		{
			return cells[n];
		}
		bool GetCell(int i, int j)
		{
			return cells[i][j];
		}
		int GetColor() { return color; }
	};

	figure* current_figure = 0;

	class S :public figure
	{
	private:
	public:
		S() : figure(2)
		{
			for (int i = 0; i < 2; i++)
				for (int j = 0; j < 2; j++)
					cells[i][j] = true;
			color = 1;
		}
		S(const S & f) :figure(f) {}
	};
	class L :public figure
	{
	public:
		L() : figure(3)
		{
			cells[0][0] = true;
			cells[0][1] = true;
			cells[1][1] = true;
			cells[2][1] = true;
			color = 2;
		}
		L(const L& f) :figure(f) {}
	};
	class LRev :public figure
	{
	public:
		LRev() : figure(3)
		{
			cells[0][1] = true;
			cells[0][2] = true;
			cells[1][1] = true;
			cells[2][1] = true;
			color = 3;
		}
		LRev(const LRev& f) :figure(f) {}
	};
	class T :public figure
	{
	public:
		T() : figure(3)
		{
			cells[0][1] = true;
			cells[1][0] = true;
			cells[1][1] = true;
			cells[1][2] = true;
			color = 4;
		}
		T(const T& f) :figure(f) {}
	};
	class I :public figure
	{
	public:
		I() : figure(4)
		{
			cells[0][1] = true;
			cells[1][1] = true;
			cells[2][1] = true;
			cells[3][1] = true;
			color = 5;
		}
		I(const I& f) :figure(f) {}
	};
	class Z :public figure //Z - "'|."
	{
	public:
		Z() : figure(3)
		{
			cells[0][1] = true;
			cells[1][0] = true;
			cells[1][1] = true;
			cells[2][0] = true;
			color = 6;
		}
		Z(const Z& f) :figure(f) {}
	};
	class ZRev :public figure //ZRev - ".|'"
	{
	public:
		ZRev() : figure(3)
		{
			cells[0][0] = true;
			cells[1][0] = true;
			cells[1][1] = true;
			cells[2][1] = true;
			color = 7;
		}
		ZRev(const ZRev& f) :figure(f) {}
	};

void ShowNextFigure();
figure* GetFigure(int figure_id);
void color_sprite(int i)
{
	switch (i)
	{
	case 0: break;
	case 1:
		field_sprite.setTextureRect(sf::IntRect(22, 24, 10, 10));
		break;
	case 2:
		field_sprite.setTextureRect(sf::IntRect(36, 24, 10, 10));
		break;
	case 3:
		field_sprite.setTextureRect(sf::IntRect(50, 24, 10, 10));
		break;
	case 4:
		field_sprite.setTextureRect(sf::IntRect(64, 24, 10, 10));
		break;
	case 5:
		field_sprite.setTextureRect(sf::IntRect(22, 38, 10, 10));
		break;
	case 6:
		field_sprite.setTextureRect(sf::IntRect(36, 38, 10, 10));
		break;
	case 8:
		field_sprite.setTextureRect(sf::IntRect(50, 38, 10, 10));
		break;
	case 7:
		field_sprite.setTextureRect(sf::IntRect(64, 38, 10, 10));
		break;
	case 9:
		field_sprite.setTextureRect(sf::IntRect(22, 52, 10, 10));
		break;
	case 10:
		field_sprite.setTextureRect(sf::IntRect(50, 52, 10, 10));
		break;
	case 11:
		field_sprite.setTextureRect(sf::IntRect(64, 52, 10, 10));
		break;
	}
}
class field
{
private:
	int field_cells[HEIGHT][WIDTH];
	int test_field_cells[HEIGHT][WIDTH];

	int current_x, current_y;
public:
	field()
	{
		for (int i = 0; i < HEIGHT; i++)
		{
			for (int j = 0; j < WIDTH; j++)
			{
				field_cells[i][j] = 0;
			}
		}
		current_x = WIDTH / 2 - 1;
		current_y = 0;
	}
	int test(action test_action)
	{
		CopyTest();
		bool test_success = true;

		int test_x = current_x;
		int test_y = current_y;

		figure test_figure(*current_figure);

		switch (test_action)
		{
		case 0:
			test_y--;
			break;
		case 1:
			test_x--;
			break;
		case 2:
			test_y++;
			break;
		case 3:
			test_x++;
			break;
		case 4:
			test_figure.RotateFigure(clockwise);
			break;
		case 5:
			test_figure.RotateFigure(anticlockwise);
			break;
		}

		for (int k = 0; k < 5 && test_success; k++)
		{
			if (test_x < 0)
			{
				for (int j = 0; j < -test_x && test_success; j++)
				{
					for (int i = 0; i < test_figure.GetSize() && test_success; i++)
					{
						if (test_figure[i][j])
							test_success = false;
					}
				}
			}

			if (test_x + test_figure.GetSize() >= WIDTH && test_success)
			{
				for (int i = 0; i < test_figure.GetSize() && test_success; i++)
				{
					for (int j = 0; j < test_x + test_figure.GetSize() - WIDTH && test_success; j++)
					{
						if (test_figure[i][test_figure.GetSize() - j - 1])
							test_success = false;
					}
				}
			}

			if ((test_action == 2 || test_action == 4 || test_action == 5) && test_success)
			{
				for (int i = 0; i < test_figure.GetSize() && test_success; i++)
				{
					int bottom_y = -1;

					for (int j = test_figure.GetSize() - 1; j >= 0; j--)
					{
						if ((bottom_y == -1) && test_figure[j][i])
							bottom_y = j;
					}
					if (field_cells[test_y + bottom_y][test_x + i] && bottom_y != -1)
						test_success = false;
					if (current_y + bottom_y + 1 >= HEIGHT && bottom_y != -1)
						test_success = false;
				}
			}

			if (test_success)
			{
				for (int i = 0; i < test_figure.GetSize() && test_success; i++)
				{
					for (int j = 0; j < test_figure.GetSize() && test_success; j++)
					{
						if (test_field_cells[test_y + i][test_x + j] && test_figure[i][j])
							test_success = false;
					}
				}
			}

			if (test_success)
				break;

			if (!test_success && (test_action == 4 || test_action == 5) && k != 4)
			{
				switch (k)
				{
				case 0:
					test_x--;
					break;
				case 1:
					test_x += 2;
					break;
				case 2:
					test_x -= 3;
					break;
				case 3:
					test_x += 4;
					break;
				}

				test_success = true;
			}
		}

		if (!test_success)
			return 0;
		if (test_action == 4 || test_action == 5)
			current_x = test_x;
		return 1;
	}
	void show()
	{
		window.clear();
		field_sprite.setTextureRect(sf::IntRect(0, 0, 10, 10));
		field_sprite.setScale(2, 2);
		for (int i = 0; i < HEIGHT+100; i++) // erasing
			for (int j = 0; j < WIDTH+100; j++)
			{
				field_sprite.setTextureRect(sf::IntRect(0, 0, 10, 10));
				field_sprite.setPosition(j * 20, i * 20+10);
				window.draw(field_sprite);
			}
		for (int i = 0; i < 11 * 2; i++) // border
		{
			field_sprite.setTextureRect(sf::IntRect(43, 9, 5, 5));
			field_sprite.setPosition(i * 10, 0);
			window.draw(field_sprite);
			field_sprite.setPosition(i * 10, HEIGHT * 20+10);
			window.draw(field_sprite);
			field_sprite.setPosition(i * 10, 510);
			window.draw(field_sprite);
		}
		for (int j = 0; j < 26 * 2; j++) // border
		{
			field_sprite.setTextureRect(sf::IntRect(43, 9, 5, 5));
			field_sprite.setPosition(0, j * 10);
			window.draw(field_sprite);
			field_sprite.setPosition(210, j * 10);
			window.draw(field_sprite);
		}
		for (int i = 0; i < HEIGHT; i++) // field
			for (int j = 0; j < WIDTH; j++)
			{
				color_sprite(field_cells[i][j]);
				
				if (field_cells[i][j])
				{
					field_sprite.setPosition(j * 20+10, i * 20+10);
					window.draw(field_sprite);
				}
			}
		figure* temp_figure = GetFigure(queue[0]);

		text_next.setString("Next \nfigure: ");
		text_next.setPosition(25, (HEIGHT+1)*20+30);
		window.draw(text_next);

		for (int i = 0; i < temp_figure->GetSize(); i++) // next figure
			for(int j = 0; j < temp_figure->GetSize(); j++)
			{
				if (temp_figure->GetCell(i,j))
				{
					color_sprite(temp_figure->GetColor());
					field_sprite.setPosition(135+j * 20, (i+HEIGHT+1)*20+30);
					window.draw(field_sprite);
				}
			}
		delete temp_figure;

		string string_score = "Score: ";
		string_score += to_string(score);
		text_score.setString(string_score);
		text_score.setPosition(25, (HEIGHT + 1) * 20 + 130);
		window.draw(text_score);

		window.display();
	}
	int AddFigure()
	{
		for (int i = 0; i < current_figure->GetSize(); i++)
		{
			for (int j = 0; j < current_figure->GetSize(); j++)
			{
				if (field_cells[current_y + i][current_x + j] && (*current_figure)[i][j] )
					return 0;
			}
		}

		for (int i = 0; i < current_figure->GetSize(); i++)
		{
			for (int j = 0; j < current_figure->GetSize(); j++)
			{
				if ((*current_figure)[i][j] && current_y + i < HEIGHT && current_x + j < WIDTH)
					field_cells[current_y + i][current_x + j] = (*current_figure).GetColor();
			}
		}
		return 1;
	}
	void DeleteFigure()
	{
		for (int i = 0; i < current_figure->GetSize(); i++)
		{
			for (int j = 0; j < current_figure->GetSize(); j++)
			{
				if ((*current_figure)[i][j])
					field_cells[current_y + i][current_x + j] = 0;
			}
		}
	}
	void down()
	{
		current_y++;
	}
	void up()
	{
		current_y--;
	}
	void left()
	{
		current_x--;
	}
	void right()
	{
		current_x++;
	}
	int IfCollision()
	{
		for (int i = 0; i < current_figure->GetSize(); i++)
		{
			int bottom_y = -1;
			for (int j = current_figure->GetSize() - 1; j >= 0; j--)
			{
				if ((bottom_y == -1) && (*current_figure)[j][i])
					bottom_y = j;
			}
			if (field_cells[current_y + bottom_y + 1][current_x + i] && bottom_y != -1)
				return 0;
			if (current_y + bottom_y + 1 == HEIGHT && bottom_y != -1)
				return 0;
		}
		return 1;
	}
	void DeleteLine(int line)
	{
		for (int i = line; i > 0; i--)
			for (int j = 0; j < WIDTH; j++)
				field_cells[i][j] = field_cells[i - 1][j];

		for (int i = 0; i < WIDTH; i++)
			field_cells[0][i] = 0;

		if (delay > min_delay)
			delay -= 25ms;
		if (delay < min_delay)
			delay = min_delay;

		score += 100 * (11 - delay.count() / 100000);
	}
	void CheckLine()
	{
		for (int i = 0; i < HEIGHT; i++)
		{
			bool if_delete = true;

			for (int j = 0; j < WIDTH; j++)
				if (!field_cells[i][j])
					if_delete = false;

			if (if_delete)
				DeleteLine(i);
		}
	}
	void CopyTest()
	{
		for (int i = 0; i < HEIGHT; i++)
			for (int j = 0; j < WIDTH; j++)
				test_field_cells[i][j] = field_cells[i][j];
	}
	void SetXY(int x, int y)
	{
		current_x = x;
		current_y = y;
	}
};

field FIELD;

figure* GetFigure(int figure_id)
{
	switch (figure_id)
	{
	case 0:
		return new S;
	case 1:
		return new L;
	case 2:
		return new LRev;
	case 3:
		return new Z;
	case 4:
		return new ZRev;
	case 5:
		return new T;
	case 6:
		return new I;
	}
}
void GetRandFig()
{
	srand(time(NULL));
	if (queue[0] == -1)
	{
		queue[0] = rand() % 7;
	}
	else
		if (current_figure)
		{
			delete current_figure;
			current_figure = 0;
		}
	current_figure = GetFigure(queue[0]);

	for (int i = 1; i < 4; i++)
	{
		queue[i] = queue[i + 1];
	}
	queue[4] = queue[0];

	for (bool if_valid_rand = false; !if_valid_rand;)
	{
		int temp_rand = rand() % 7;
		if_valid_rand = true;
		for (int i = 1; i < 5; i++)
		{
			if (temp_rand == queue[i])
				if_valid_rand = false;
		}
		if (if_valid_rand)
			queue[0] = temp_rand;
	}
}
int Collision()
{
	if (!FIELD.IfCollision())
	{
		FIELD.SetXY(WIDTH / 2 - 1, 0);
		GetRandFig();
		figures_count++;

		if (!(figures_count % 5) && figures_count > 0 && delay > min_delay)
			delay -= 10ms;
		if (delay < min_delay)
			delay = min_delay;
		score += 5 * (11 - delay.count() / 100000);

		FIELD.CheckLine();
		return 1;
	}
	return 0;
}
int Down()
{
	FIELD.DeleteFigure();
	bool if_down = false;

	if (FIELD.test(DOWN))
	{
		FIELD.down();
		if_down = true;
	}

	FIELD.AddFigure();

	if (!if_down)
		return 2;
	return 1;
}
void Left()
{
	bool if_left = false;

	FIELD.DeleteFigure();
	if (FIELD.test(LEFT))
	{
		FIELD.left();
		if_left = true;
	}
	FIELD.AddFigure();

	if (if_left)
		FIELD.show();
}
void Right()
{
	bool if_right = false;

	FIELD.DeleteFigure();
	if (FIELD.test(RIGHT))
	{
		FIELD.right();
		if_right = true;
	}
	FIELD.AddFigure();

	if (if_right)
		FIELD.show();
}
void Rotate(rot r)
{
	bool if_rotate = false;

	FIELD.DeleteFigure();

	action rot_type;
	if (r == clockwise)
		rot_type = ROT_CLOCKWISE;
	else
		rot_type = ROT_ANTICLOCKWISE;

	if (FIELD.test(rot_type))
	{
		current_figure->RotateFigure(r);
		if_rotate = true;
	}
	FIELD.AddFigure();
	if (if_rotate)
		FIELD.show();
}
void GameOver()
{
	if_gameover = 1;
	window.clear();
	sf::Texture menuTexture1, menuTexture3, menuTexture4, menuTexture5;
	menuTexture1.loadFromFile("images/444.png");
	menuTexture3.loadFromFile("images/555.png");
	menuTexture4.loadFromFile("images/222.png");
	menuTexture5.loadFromFile("images/333.png");
	sf::Sprite menu1(menuTexture1), menu3(menuTexture3), menu4(menuTexture4), menu5(menuTexture5);
	isMenu = 1;
	int menuNum = 0;
	menu1.setPosition(10, 30);
	menu3.setPosition(10, 150);
	menu4.setPosition(10, 300);
	menu5.setPosition(10, 400);

	text_score.setString(to_string(score));
	text_score.setPosition(20, 215);
	text_score.setCharacterSize(40);

	//while (isMenu)
	{
		menu4.setColor(sf::Color::White);
		menu5.setColor(sf::Color::White);
		//menuNum = 0;
		window.clear(sf::Color(129, 181, 221));
		if (sf::IntRect(10, 300, 200, 40).contains(sf::Mouse::getPosition(window))) { menu4.setColor(sf::Color::Blue); menuNum = 4; }
		if (sf::IntRect(10, 400, 200, 40).contains(sf::Mouse::getPosition(window))) { menu5.setColor(sf::Color::Blue); menuNum = 5; }
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (menuNum == 4)
			{
				isMenu = false;

				if_gameover = true;
				if (current_figure)
				{
					delete current_figure;
					current_figure = 0;
				}
				FIELD.field::field();
			}
			if (menuNum == 5) 
			{
				window.close();
				isMenu = false;
				if_exit = true;
				if_gameover = true;
			}
		}
		window.draw(menu1);
		window.draw(menu3);
		window.draw(menu4);
		window.draw(menu5);
		window.draw(text_score);
		window.display();
	}

	text_score.setCharacterSize(30);
}
void Fall(mutex & mut)
{
	for (;;)
	{
		while (!mut.try_lock());
		if (if_gameover)
		{
			mut.unlock();
			return;
		}
		//Down();
		//FIELD.show();

		mut.unlock();

		this_thread::sleep_for(delay);
		while (!mut.try_lock());
		if (if_gameover)
		{
			mut.unlock();
			return;
		}
		if (Collision())
		{
			if (!FIELD.AddFigure())
			{
				//FIELD.show();
				if_gameover = 1;
				GameOver();
				mut.unlock();
				return;
			}
			//FIELD.show();
		}
		mut.unlock();
	}
}
/*int TetrisInterface()
{
	system("cls");
	cout << "        TETRIS" << endl << endl << "Press ENTER to start or continue" << endl
		<< "Press X to finish the game" << endl << endl
		<< setw(22) << "DOWN: " << "'S' or Arrow Down" << endl
		<< setw(22) << "LEFT: " << "'A' or Arrow Left" << endl
		<< setw(22) << "RIGHT: " << "'D' or Arrow Right" << endl
		<< "ROTATE " << setw(15) << "CLOCKWISE: " << "'E' or Arrow Up" << endl
		<< setw(22) << "ROTATE ANTICLOCKWISE: " << "'Q'" << endl << endl;
	char c;
	for (c = '0'; c != 13 && c != 'X' && c != 'x'; c = _getch());
	if (c != 13)
		return 0;
	return 1;
}*/
void Restart()
{
	score = 0;
	if_gameover = false;
	figures_count = 0;
	delay = 1000ms;
	for (int i = 0; i < 5; i++)
		queue[i] = -1;
	if (current_figure != 0)
	{
		delete current_figure;
		current_figure = 0;
	}
}

int menu(sf::RenderWindow& window) 
{
	window.clear();
	sf::Texture menuTexture1, menuTexture3, menuTexture4;
	if(!if_not_first_menu) 
		menuTexture1.loadFromFile("images/111.png");
	else
	{
		menuTexture1.loadFromFile("images/666.png");
		menuTexture4.loadFromFile("images/222.png");
	}
	menuTexture3.loadFromFile("images/333.png");
	sf::Sprite menu1(menuTexture1), menu3(menuTexture3),menu4(menuTexture4);
	isMenu = 1;
	int menuNum = 0;
	menu1.setPosition(10, 30);
	menu4.setPosition(10, 150);
	menu3.setPosition(10, 270);

	//while (isMenu)
	{
		menu1.setColor(sf::Color::White);
		menu3.setColor(sf::Color::White);
		menu4.setColor(sf::Color::White);
		//menuNum = 0;
		window.clear(sf::Color(129, 181, 221));
		if (sf::IntRect(10, 30, 200, 40).contains(sf::Mouse::getPosition(window))) { menu1.setColor(sf::Color::Blue); menuNum = 1; }
		if (sf::IntRect(10, 150, 200, 40).contains(sf::Mouse::getPosition(window))) { menu4.setColor(sf::Color::Blue); menuNum = 4; }
		if (sf::IntRect(10, 270, 200, 40).contains(sf::Mouse::getPosition(window))) { menu3.setColor(sf::Color::Blue); menuNum = 3; }
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (menuNum == 4 && if_not_first_menu) { isMenu = false; if_not_first_menu = 1; return 2; }
			if (menuNum == 1) { isMenu = false; if_not_first_menu = 1; return 1; }
			if (menuNum == 3) 
			{ 
				window.close(); 
				isMenu = false;
				if_exit = true;
				if_gameover = true; 
				return 0; 
			}
		}
		window.draw(menu1);
		window.draw(menu3);
		window.draw(menu4);
		window.display();
	}
}

void main()
{
	char command;
	field_image.loadFromFile("images/tetris.png");
	field_texture.loadFromImage(field_image);
	field_sprite.setTexture(field_texture);
	//window.setFramerateLimit(10);

	font.loadFromFile("17112.otf");
	text_next.setFillColor(sf::Color::Black);
	text_score.setFillColor(sf::Color::Black);

	sf::Clock clock, key_repeat, key_delay;
	float timer = 0, fall_delay = 0.3, timer_key_repeat = 0, timer_key_delay = 0;
	bool if_key_repeat = false;
	char current_action = 0;

	while (window.isOpen())
	{
		if (if_exit)
			window.close();

		while (!if_exit)
		{
			Restart();
			/*if (!TetrisInterface())
			{
				GameOver();
				if_gameover = true;
				continue;
			}*/
			GetRandFig();
			FIELD.AddFigure();

			for (; !if_exit;)
			{
				window.setSize(sf::Vector2u(220, 520));
				bool if_window_action = false;
				//double time = clock.getElapsedTime().asMilliseconds();
				timer += clock.getElapsedTime().asMilliseconds();
				clock.restart();
				Sleep(1);
				if (timer > delay.count() && if_not_first_menu && !isMenu)
				{
					//cout << endl << timer << " " << delay.count() << endl << endl;
					timer = 0;
					if_window_action = true;
					command = 'S';
				}

				sf::Event event;
				while (window.pollEvent(event) || !if_not_first_menu)
				{
					if (event.type == sf::Event::Closed)
					{
						window.close();
						if_exit = true;
						if_gameover = true;
					}

					if (!if_not_first_menu)
					{
						menu(window);
						if (if_exit)
							break;
					}

					if (if_gameover)
					{
						GameOver();
					}
					if (event.type == sf::Event::KeyPressed || isMenu && if_not_first_menu && !if_gameover)
					{

						if (!if_key_repeat || timer_key_delay >= 50 && if_key_repeat && timer_key_repeat > 400 || isMenu && if_not_first_menu)
						{
							if (if_key_repeat == false)
							{
								timer_key_repeat = 0;
								timer_key_delay = 0;
								key_repeat.restart();
								key_delay.restart();
							}
							if ((event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A) && !isMenu)
							{
								if (current_action != 'A' && if_key_repeat)
								{
									if_key_repeat = false;
									timer_key_repeat = 0;
								}
								command = 'A';
								if_window_action = true;
								current_action = 'A';
							}
							if ((event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D) && !isMenu)
							{
								if (current_action != 'D' && if_key_repeat)
								{
									if_key_repeat = false;
									timer_key_repeat = 0;
								}
								command = 'D';
								if_window_action = true;
								current_action = 'D';
							}
							if ((event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) && !isMenu)
							{
								if (current_action != 'S' && if_key_repeat)
								{
									if_key_repeat = false;
									timer_key_repeat = 0;
								}
								command = 'S';
								if_window_action = true;
								current_action = 'S';
								timer = 0;
							}
							if ((event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::E) && !isMenu)
							{
								if (current_action != 'E' && if_key_repeat)
								{
									if_key_repeat = false;
									timer_key_repeat = 0;
								}
								command = 'E';
								if_window_action = true;
								current_action = 'E';
							}
							if (event.key.code == sf::Keyboard::Escape || isMenu && if_not_first_menu && !if_gameover)
							{
								if (menu(window) == 2)
								{
									if_gameover = true;
									if (current_figure)
									{
										delete current_figure;
										current_figure = 0;
									}
									FIELD.field::field();
								}
								timer = 0;
								clock.restart();

							}

							timer_key_delay = 0;
							//cout << " ";
						}
						if_key_repeat = true;
						timer_key_repeat += key_repeat.getElapsedTime().asMilliseconds();
						key_repeat.restart();
						//cout <<"timer key repeat"<< timer_key_repeat<<endl;
						timer_key_delay += key_delay.getElapsedTime().asMilliseconds();
						//cout << "timer key delay  " << timer_key_delay<<endl;
						//timer_key_delay = 0;
						key_delay.restart();
						//if (timer_key_repeat > 550)
							//timer_key_repeat = 500;
					}
					if (event.type == sf::Event::KeyReleased && if_key_repeat)
					{
						if_key_repeat = false;
						key_repeat.restart();
						timer_key_repeat = 0;
						key_delay.restart();
						timer_key_delay = 0;
						//cout << timer_key_repeat;
					}
				}
				if (if_gameover)
				{
					//if (thr.joinable())
					//	thr.join();
					//break;
				}
				if (_kbhit() || if_window_action)
				{
					if (!if_window_action)
					{
						command = static_cast<char>(_getch());
						//char command = 's';
						if (command == -32)
						{
							command = static_cast<char>(_getch());
							switch (command)
							{
							case 72:
								command = 'E';
								break;
							case 80:
								command = 'S';
								break;
							case 75:
								command = 'A';
								break;
							case 77:
								command = 'D';
								break;
							}
						}
					}
					if_window_action = false;

					if (if_gameover)
					{
						break;
					}
					switch (command)
					{
					case 'S':
					case 's':
						if (Down() == 2)
						{
							Collision();
							if (!FIELD.AddFigure())
							{
								if_gameover = true;
								GameOver();
								break;
							}
						}
						FIELD.show();
						break;
					case 'A':
					case 'a':
						Left();
						break;
					case 'D':
					case 'd':
						Right();
						break;
					case 'Q':
					case 'q':
						Rotate(anticlockwise);
						break;
					case 'E':
					case 'e':
						Rotate(clockwise);
						break;
						/*case 'P':
						case 'p':
							if (!TetrisInterface())
							{
								if_gameover = true;
								GameOver();
							}
							break;*/
					}
					if (if_gameover)
					{
						continue;
					}
				}
			}
		}
	}
}
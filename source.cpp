//В конце разработки надо будет как то распихать всё это чудо по файлам и систематизировать, а также понаделать анимаций + звуков
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<algorithm>
//#include <cstdlib>//если рандом не захочет работать
#include<ctime>
#include<string>
#include<cstring>
#include<map>
//нужно .h и .cpp файлы делать
using namespace sf;
using namespace std;

enum class PGS { free, active_ability, animation, damage, particle_flying, activating_active, dealing_damage, activating_Altactive, Altactive_ability, conceding };
//PGS = Possible Global States, but i am too lazy to write it every time

vector<int> answer(4);
map<string, Texture> textures;
PGS globalState = PGS::free;
vector<pair<string, string>> allPassives = { {"poisoned","status"}, {"Poison touch", "attack"}, {"running", "status"}, {"runrunrun", "turn start"} };
int select = -1;
int damaged = -1;
int nowtargetx = -1;
int nowtargety = -1;
bool nowBotTurn = false;
bool concede = false;
const int w = 11;
const int h = 6;
vector<vector<string>> battlefieldState(h, vector<string>(w, "free"));
vector<Sprite> turnTracker;
vector<RectangleShape> turnTrackerWindows;
vector<bool> trackerFriends;


struct Unit;

vector<Unit> us;
vector<Unit> they;

struct StatBlock {
	int msChange = 0;
	int dmgChange = 0;
	int hpmaxChange = 0;
	int tickdmg = 0;
	int initiativeChange = 0;
	void applyDelta(Unit& owner);
	void disapplyDelta(Unit& owner);
	void tick(Unit& owner, int id);
	StatBlock(int ms, int dmg, int hpmax, int tickdmgA, int initi);
	StatBlock() {}
};

struct Passive {
	string name;
	int id;
	string type;
	int time;
	bool (*condition)(Unit&) = [](Unit& owner) {return true; };//бывашая rtechicallythetruth
	string trigger;
	string target = "";
	int dmg = 0;//для триггеринг
	vector<Passive> effects_enemy = {};//для триггеринг
	vector<Passive> effects_friends = {};
	StatBlock enemyStats;//для триггеринг
	StatBlock friendStats;
	bool operator==(int& another) {
		return this->id == another;
	}
	const bool operator==(const Passive& another) {
		return this->id == another.id;
	}
	bool operator==(Passive& another) {
		return this->id == another.id;
	}
	void triggering(Unit& owner, int targetx, int targety, vector<Unit>& zhertvi);
	void triggering(Unit& owner);
	void apply(Unit& owner);
	void disapply(Unit& owner);
	void tick(Unit& owner);
	Passive() {};
};

vector<Passive> statuses;

struct ActiveAbility;
struct Particle {
	string name;
	RectangleShape txt;
	int startSize;
	int endSize;
	int nowmovex;
	int nowmovey;
	int stepSize;
	int numberOfIterations;
	int stepMoveX;
	int stepMoveY;
	int iters;
	int deltaX;
	int deltaY;

	void draw(int targetx, int targety, vector<Unit>& us);
	void draw(bool isActive);
};

struct Unit {
	int fireballNumber;
	int healNumber;
	int altActive;
	bool isRangeUnit = false;
	int attackRange;
	Particle bullet;
	bool skippedTurnThisRound = false;
	bool hasActive = false;
	int active;
	Texture txtt;
	int hp;
	int ms;
	int dmg;
	int mindmg;
	int maxdmg;
	int deltadmg = 2;
	bool alive = true;
	int positionx;
	int positiony;
	int nowmovex;
	int nowmovey;
	int initiative;
	int hpMax;
	bool hasUnappliedStatus = false;
	Sprite txt;
	int defaultActiveCooldown;
	int defaultAltActiveCooldown;
	int iters;
	int stepMoveX;
	int stepMoveY;
	int cooldown;
	int altCooldown;
	Sprite activeButton;
	Sprite altActiveButton;
	vector<Passive> statuses = {};
	vector<Passive> passives_whenTurnStart_base = {};
	vector<Passive> passives_whenTurnStart = {};
	vector<Passive> passives_whenAttack_base = {};
	vector<Passive> passives_whenAttack = {};
private:
	String type;
public:
	String getType() {
		return type;
	}
	void setType(string& dano) {
		type = dano;
		txtt = textures[dano];
		if (type == "dog" || type == "we") {	
			battlefieldState[positiony][positionx] = "friend";
		}
		else {
			battlefieldState[positiony][positionx] = "enemy";
		}
		txt.setScale(0.9, 0.9);
		txt.move(positionx * 150.f + 140.f, positiony * 150.f + 10.f);
	}
	vector<int> Status_duration;//-1, если статуса нет, иначе рил длительность, i - id
	Unit() {
		fireballNumber = 0;
		healNumber = 0;
		attackRange = -1;
		isRangeUnit = false;
		skippedTurnThisRound = false;
		activeButton.setPosition(Vector2f(1605.f, 925.f));
		altActiveButton.setPosition(Vector2f(1770.f, 925.f));
		Status_duration = { -1,-1,-1,-1, -1, -1 };
		hasActive = false;
		active = -1;
		altActive = -1;
		defaultActiveCooldown = -1;
		defaultAltActiveCooldown = -1;
		cooldown = 0;
	}
	Unit(int hpn, int msn, int dmgn, int posx, int posy, int ini) {
		fireballNumber = 0;
		healNumber = 0;
		attackRange = -1;
		isRangeUnit = false;
		skippedTurnThisRound = false;
		activeButton.setPosition(Vector2f(1605.f, 925.f));
		altActiveButton.setPosition(Vector2f(1770.f, 925.f));
		Status_duration = { -1,-1,-1,-1, -1, -1 };
		hasActive = false;
		active = -1;
		altActive = -1;
		defaultActiveCooldown = -1;
		defaultAltActiveCooldown = -1;
		cooldown = 0;
		hp = hpn;
		ms = msn;
		dmg = dmgn;
		mindmg = max(0, dmgn - deltadmg);
		maxdmg = dmgn + deltadmg;
		positionx = posx;
		positiony = posy;
		initiative = ini;
		hpMax = hp;
	}
	Unit(int posx, int posy) {
		attackRange = -1;
		isRangeUnit = false;
		skippedTurnThisRound = false;
		activeButton.setPosition(Vector2f(1605.f, 925.f));
		altActiveButton.setPosition(Vector2f(1770.f, 925.f));
		Status_duration = { -1,-1,-1,-1, -1, -1 };
		hasActive = false;
		active = -1;
		defaultActiveCooldown = -1;
		defaultAltActiveCooldown = -1;
		cooldown = 0;
		positionx = posx;
		positiony = posy;
		hpMax = hp;
	}
	void turnStartPassives() {
		for (int i = 0; i < passives_whenTurnStart.size(); i++) {
			passives_whenTurnStart[i].triggering(*this);
		}
	}
	void attackPassives(int targetx, int targety, vector<Unit>& zhertvi) {
		for (int i = 0; i < passives_whenAttack.size(); i++) {
			passives_whenAttack[i].triggering(*this, targetx, targety, zhertvi);
		}
	}
	void tick() {
		if (cooldown > 0) {
			cooldown--;
		}
		for (int i = 0; i < Status_duration.size(); i++) {
			if (Status_duration[i] >= 0) {
				statuses[i].tick(*this);
			}
		}
	}
	void useActive() {
		globalState = PGS::active_ability;
	}
	void useAltActive() {
		globalState = PGS::Altactive_ability;
	}
	void takedmg(int tika) {
		hp -= tika;
		if (hp > hpMax) {
			hp = hpMax;
		}
		else if (hp <= 0) {
			alive = false;
			battlefieldState[positiony][positionx] = "free";
			hp = 1;
		}
	}
	void takedmg(int tika, bool barrier) {
		hp -= tika;
		if (hp > hpMax && !barrier) {
			hp = hpMax;
		}
		else if (hp <= 0) {
			alive = false;
			battlefieldState[positiony][positionx] = "free";
			hp = 1;
		}
	}
	void takedmg(int midmg, int madmg, bool barrier) {
		hp -= (midmg + (rand() / ((RAND_MAX + 1u) / (madmg - midmg + 1))));
		if (hp > hpMax && !barrier) {
			hp = hpMax;
		}
		else if (hp <= 0) {
			alive = false;
			battlefieldState[positiony][positionx] = "free";
			hp = 1;
		}
	}
	void draw(RenderWindow& wind) {
		if (alive == true) {
			txt.setTexture(txtt);
			if (Status_duration[0] > 0) {
				txt.setColor(Color(0, 100, 0, 255));
			}
			wind.draw(txt);
			if (hasActive && globalState == PGS::free && !nowBotTurn && select != -1 && fireballNumber > 0) {
				if (cooldown == 0) {
					activeButton.setColor(Color::White);
					activeButton.setTexture(textures["fireballButton"]);
				}
				else {
					activeButton.setColor(Color::Black);
				}
				wind.draw(activeButton);
			}
			if (hasActive && globalState == PGS::active_ability) {
				activeButton.setTexture(textures["fireballButton"]);
				altActiveButton.setTexture(textures["healButton"]);
				wind.draw(activeButton);
			}
			if (hasActive && globalState == PGS::free && !nowBotTurn && select != -1 && healNumber > 0) {
				if (altCooldown == 0) {
					altActiveButton.setColor(Color::White);
					altActiveButton.setTexture(textures["healButton"]);
				}
				else {
					altActiveButton.setColor(Color::Black);
				}
				wind.draw(altActiveButton);
			}
		}
	}
	bool operator>(const Unit& another) {
		return initiative > another.initiative;
	}
	bool operator>=(const Unit& another) {
		return initiative >= another.initiative;
	}
	bool operator<(const Unit& another) {
		return initiative < another.initiative;
	}
	bool operator<=(const Unit& another) {
		return initiative <= another.initiative;
	}
	void move(int dx, int dy) {
		globalState = PGS::animation;
		nowmovex = dx * 150.f;
		nowmovey = dy * 150.f;
		iters = max(abs(nowmovex) / 15, abs(nowmovey) / 15);
		if (iters != 0) {
			stepMoveX = nowmovex / iters;
			stepMoveY = nowmovey / iters;
		}
		else {
			stepMoveX = 0;
			stepMoveY = 0;
		}
		positionx += dx;
		positiony += dy;
	}
	void move() {
		iters--;
		txt.move(stepMoveX, stepMoveY);
		if (iters <= 0) {
			globalState = PGS::free;
			select = -1;
		}
	}
	void attack(int dx, int dy) {
		globalState = PGS::damage;
		if (!isRangeUnit) {
			nowmovex = dx * 150.f;
			nowmovey = dy * 150.f;
			iters = max(abs(nowmovex) / 15, abs(nowmovey) / 15);
			if (iters != 0) {
				stepMoveX = nowmovex / iters;
				stepMoveY = nowmovey / iters;
			}
			else {
				stepMoveX = 0;
				stepMoveY = 0;
			}
			positionx += dx;
			positiony += dy;
		}
		else {
			if (nowBotTurn) {
				bullet.draw(positionx + dx, positiony + dy, they);
			}
			else {
				bullet.draw(positionx + dx, positiony + dy, us);
			}
		}
	}
	void attack(Unit& zhertva, vector<Unit>& attackers, vector<Unit>& zhertvi) {
		if (!isRangeUnit) {
			iters--;
			txt.move(stepMoveX, stepMoveY);
			if (iters <= 0) {
				attackers[select].attackPassives(zhertvi[damaged].positionx, zhertvi[damaged].positiony, zhertvi);
				zhertva.takedmg(dmg);
				damaged = -1;
				select = -1;
				globalState = PGS::free;
			}
		}
		else {
			bullet.draw(false);
			if (globalState == PGS::dealing_damage) {
				attackers[select].attackPassives(zhertvi[damaged].positionx, zhertvi[damaged].positiony, zhertvi);
				zhertva.takedmg(dmg);
				damaged = -1;
				select = -1;
				globalState = PGS::free;
			}
		}
	}
};
Vector2f sumOfVectors2f(const Vector2f& a, const Vector2f& b) {
	return Vector2f(a.x + b.x, a.y + b.y);
}

bool contMouse(RectangleShape& rect) {
	return rect.getGlobalBounds().contains(Mouse::getPosition().x, Mouse::getPosition().y);
}
bool contMouse(Sprite& rect) {
	return rect.getGlobalBounds().contains(Mouse::getPosition().x, Mouse::getPosition().y);
}
int sqr(int a) {
	return a * a;
}
int pythagor(int a, int b) {
	return sqr(a) + sqr(b);
}
bool isEnemyNear(Unit& owner) {
	int x = owner.positionx;
	int y = owner.positiony;
	string searching = "";
	if (battlefieldState[y][x] == "enemy") {
		searching = "friend";
	}
	else {
		searching = "enemy";
	}
	pair<int, int> hodi[] = { {0,1}, {0, -1},{1, 0}, {-1, 0} };
	for (int i = 0; i < 4; i++) {
		if (x + hodi[i].first >= 0 && x + hodi[i].first < w && y + hodi[i].second >= 0 && y + hodi[i].second < h && battlefieldState[y + hodi[i].second][x + hodi[i].first] == searching) {
			return true;
		}
	}
	return false;
}

void StatBlock::applyDelta(Unit& owner) {
	owner.ms = max(0, owner.ms + msChange);
	owner.dmg = max(0, owner.dmg + dmgChange);
	//owner.hpMax = max(0,owner.hpMax + hpmaxChange);
	owner.initiative = max(0, owner.initiative + initiativeChange);
}
void StatBlock::disapplyDelta(Unit& owner) {
	owner.ms = max(0, owner.ms - msChange);
	owner.dmg = max(0, owner.dmg - dmgChange);
	//owner.hpMax =  max(0, owner.hpMax - hpmaxChange);
	owner.initiative = max(0, owner.initiative - initiativeChange);
}
void StatBlock::tick(Unit& owner, int id) {
	owner.takedmg(tickdmg);
	owner.Status_duration[id]--;
	if (owner.Status_duration[id] == 0) {
		this->disapplyDelta(owner);
		owner.Status_duration[id] = -1;
	}
}
StatBlock::StatBlock(int ms, int dmg, int hpmax, int tickdmgA, int initi) {
	msChange = ms;
	dmgChange = dmg;
	hpmaxChange = hpmax;
	tickdmg = tickdmgA;
	initiativeChange = initi;
}

void Passive::triggering(Unit& owner, int targetx, int targety, vector<Unit>& zhertvi) {
	bool possible = condition(owner);
	if (possible && type == "ability" && target == "area") {
		//тут будет срабатывание на всех в области, но я ленивый (пока что)
		return;
	}
	if (possible && type == "ability" && target == "self") {
		for (int i = 0; i < effects_friends.size(); i++) {
			if (owner.Status_duration[effects_friends[i].id] == -1) {
				owner.Status_duration[effects_friends[i].id] = time;
				effects_friends[i].apply(owner);
			}
			else {
				owner.Status_duration[effects_friends[i].id] += time;
			}
		}
		return;
	}
	if (possible && type == "ability" && target == "attacked") {
		for (int i = 0; i < zhertvi.size(); i++) {
			if (zhertvi[i].positionx == targetx && zhertvi[i].positiony == targety && zhertvi[i].alive == true) {
				for (int j = 0; j < effects_enemy.size(); j++) {
					if (zhertvi[i].Status_duration[effects_enemy[j].id] == -1) {
						effects_enemy[j].apply(zhertvi[i]);
					}
					else {
						zhertvi[i].Status_duration[effects_enemy[j].id] += time;
					}
				}
			}
		}
	}
}
void Passive::triggering(Unit& owner) {
	bool possible = condition(owner);
	if (possible && time == 0 && type == "ability" && target == "area") {
		//тут будет срабатывание на всех в области, но я ленивый (пока что)
		return;
	}
	if (possible && type == "ability" && target == "self") {
		for (int i = 0; i < effects_friends.size(); i++) {
			effects_friends[i].apply(owner);
		}
		return;
	}
}
void Passive::apply(Unit& owner) {
	if (type == "status") {
		if (owner.Status_duration[id] == -1) {
			friendStats.applyDelta(owner);
			owner.Status_duration[id] = time;
		}
		else {
			owner.Status_duration[id] += time;
		}
		return;
	}
	if (type == "ability") {
		friendStats.applyDelta(owner);
		for (int i = 0; i < effects_friends.size(); i++) {
			if (effects_friends[i].trigger == "attack") {
				if (find(owner.passives_whenAttack.begin(), owner.passives_whenAttack.end(), effects_friends[i]) == owner.passives_whenAttack.end()) {
					owner.passives_whenAttack.push_back(effects_friends[i]);
				}
			}
			if (effects_friends[i].trigger == "turn start") {
				if (find(owner.passives_whenTurnStart.begin(), owner.passives_whenTurnStart.end(), effects_friends[i]) == owner.passives_whenTurnStart.end()) {
					owner.passives_whenTurnStart.push_back(effects_friends[i]);
				}
			}
		}
		return;
	}
}
void Passive::tick(Unit& owner) {
	if (type == "status") {
		friendStats.tick(owner, id);
		return;
	}
}

void Particle::draw(int targetx, int targety, vector<Unit>& us) {
	globalState = PGS::particle_flying;
	nowmovex = (targetx - us[select].positionx) * 150.f;
	nowmovey = (targety - us[select].positiony) * 150.f;
	nowmovex += deltaX;
	nowmovey += deltaY;
	iters = max(abs(nowmovex) / 15, abs(nowmovey) / 25);
	stepSize = (endSize - startSize) / iters;
	stepMoveX = nowmovex / iters;
	stepMoveY = nowmovey / iters;
	txt.setSize(Vector2f(startSize, startSize));
	txt.setOrigin(Vector2f(startSize / 2, startSize / 2));
	txt.setPosition(us[select].txt.getPosition());
}
void Particle::draw(bool isActive) {
	txt.move(stepMoveX, stepMoveY);
	iters--;
	if (iters == 0) {
		if (isActive) {
			globalState = PGS::activating_active;
		}
		else {
			globalState = PGS::dealing_damage;
		}
	}
	txt.setSize(txt.getSize() + Vector2f(stepSize, stepSize));
}


struct ActiveAbility {
	string name;
	Particle txt;
	int id;
	int range;
	int area;//радиус от клетки таргета (те для три на три area = 1)
	string target;//can be:earth, friend, enemy, nontarget
	vector<Passive> removedStatus = {};
	vector<Passive> addedStatus = {};
	int hpChange;
	bool isBarrier = false;

	bool operator==(ActiveAbility& another) {
		return id == another.id;
	}
	bool operator==(int& another) {
		return id == another;
	}
	void activate(int targetx, int targety, vector<Unit>& us) {
		txt.draw(targetx, targety, us);
		nowtargetx = targetx;
		nowtargety = targety;
	}
	void activate(Unit& owner) {
		if (target == "self") {
			us[select].takedmg(hpChange, isBarrier);
		}
		else if (target == "earth") {
			for (int y = max(0, nowtargety - area); y < min(h, nowtargety + area); y++) {
				for (int x = max(0, nowtargetx - area); x < min(w, nowtargetx + area); x++) {
					for (int i = 0; i < they.size(); i++) {
						if (they[i].alive == true && they[i].positionx == x && they[i].positiony == y) {
							they[i].takedmg(hpChange, isBarrier);
						}
					}
					for (int i = 0; i < us.size(); i++) {
						if (us[i].alive == true && us[i].positionx == x && us[i].positiony == y) {
							us[i].takedmg(hpChange, isBarrier);
						}
					}
				}
			}

		}
		nowtargetx = -1;
		nowtargety = -1;
		select = -1;
		globalState = PGS::free;
	}
};

vector<ActiveAbility> actives;

struct StatsTable {
	RectangleShape bg;
	Text hp;
	Text ms;
	Text dmg;
	Text ini;
	Text active;
	Text passive;
	StatsTable(Font& font) {
		hp.setFont(font);
		ms.setFont(font);
		dmg.setFont(font);
		ini.setFont(font);
		active.setFont(font);
		passive.setFont(font);
		bg.setFillColor(Color::Blue);
		bg.setSize(Vector2f(750.f, 300.f));
		bg.setOutlineColor(Color::White);
		bg.setOutlineThickness(5);
	}
	void setContent(Unit& dano) {
		hp.setString("HP: " + to_string(dano.hp) + "/" + to_string(dano.hpMax));
		ms.setString("Movespeed: " + to_string(dano.ms));
		dmg.setString("Damage: " + to_string(dano.dmg));
		ini.setString("Initiative: " + to_string(dano.initiative));
		if (dano.hasActive) {
			string activeString = "";
			activeString += "Active: " + actives[dano.active].name + " (" + to_string(dano.fireballNumber) + ")";
			if (dano.altActive != -1) {
				activeString += ", " + actives[dano.altActive].name + " (" + to_string(dano.healNumber) + "), ";
			}
			active.setString(activeString);
		}
		else {
			active.setString("Active: none");
		}
		string passiveString = "";
		for (size_t i = 0; i < dano.passives_whenAttack.size(); i++) {
			if (passiveString != "") {
				passiveString += ", ";
			}
			passiveString += dano.passives_whenAttack[i].name;
		}
		for (size_t i = 0; i < dano.passives_whenTurnStart.size(); i++) {
			if (passiveString != "") {
				passiveString += ", ";
			}
			passiveString += dano.passives_whenTurnStart[i].name;
		}
		if (passiveString == "") {
			passive.setString("Passives: none");
		}
		else {
			passive.setString("Passives: " + passiveString);
		}
	}
	void draw(Unit& dano, RenderWindow& window) {
		this->setContent(dano);
		int xsign = Mouse::getPosition().x > 960 ? -1 : 0;
		int ysign = Mouse::getPosition().y > 540 ? -1 : 0;

		bg.setPosition(Vector2f(Mouse::getPosition().x + 750 * xsign, Mouse::getPosition().y + 300 * ysign));
		hp.setPosition(sumOfVectors2f(bg.getPosition(), Vector2f(5, 5)));
		ms.setPosition(sumOfVectors2f(bg.getPosition(), Vector2f(5, 50)));
		dmg.setPosition(sumOfVectors2f(bg.getPosition(), Vector2f(5, 95)));
		ini.setPosition(sumOfVectors2f(bg.getPosition(), Vector2f(5, 140)));
		passive.setPosition(sumOfVectors2f(bg.getPosition(), Vector2f(5, 185)));
		active.setPosition(sumOfVectors2f(bg.getPosition(), Vector2f(5, 230)));

		window.draw(bg);
		window.draw(hp);
		window.draw(ms);
		window.draw(dmg);
		window.draw(ini);
		window.draw(passive);
		window.draw(active);

	}
};

struct ConcedeMenu {
	Sprite fight;
	Sprite run;
	RectangleShape bg;
	Text question;
	ConcedeMenu() {}
	ConcedeMenu(Font& font) {
		question.setFont(font);
		question.setString("Are you really want to concede?");
		question.setPosition(Vector2f(485.f, 447.f));
		fight.setTexture(textures["noButton"]);
		fight.setPosition(Vector2f(485.f, 482.f));
		run.setTexture(textures["yesButton"]);
		run.setPosition(Vector2f(1283.f, 482.f));
		bg.setOutlineThickness(5);
		bg.setOutlineColor(Color::White);
		bg.setFillColor(Color::Blue);
		bg.setSize(Vector2f(960.f, 195.f));
		bg.setPosition(Vector2f(480.f, 442.f));
	}
	void draw(RenderWindow& window) {
		window.draw(bg);
		window.draw(question);
		window.draw(fight);
		window.draw(run);
	}
	bool playerRun() {
		if (contMouse(run)) {
			return true;
		}
		return false;
	}
};
ConcedeMenu crossroads;

void chooseSelect(int& pointer_us, int& pointer_they) {
	if (pointer_they >= they.size() && pointer_us >= us.size()) {
		pointer_they = 0;
		pointer_us = 0;
		for (size_t i = 0; i < us.size(); i++) {
			us[i].skippedTurnThisRound = false;
		}
	}
	else if (pointer_they >= they.size() && pointer_us < us.size()) {
		nowBotTurn = false;
		select = pointer_us;
		pointer_us++;
	}
	else if (pointer_us >= us.size() && pointer_they < they.size()) {
		nowBotTurn = true;
		select = pointer_they;
		pointer_they++;
	}
	else if (they[pointer_they] > us[pointer_us]) {
		nowBotTurn = true;
		select = pointer_they;
	}
	else if (they[pointer_they] <= us[pointer_us]) {
		nowBotTurn = false;
		select = pointer_us;
		pointer_us++;
	}
	else {
		nowBotTurn = true;
		select = pointer_they;
		pointer_they++;
	}
}
void activateTurnStartOfSelected() {
	if (!nowBotTurn) {
		us[select].tick();
		us[select].turnStartPassives();
	}
	else {
		they[select].tick();
		they[select].turnStartPassives();
	}
}
bool usAttackCondition(int i, vector<vector<RectangleShape>>& battlefield) {
	bool inRange;
	if (!us[select].isRangeUnit) {
		inRange = pythagor(they[i].positiony - us[select].positiony, they[i].positionx - us[select].positionx) <= sqr(us[select].ms + 1);
	}
	else {
		inRange = pythagor(they[i].positiony - us[select].positiony, they[i].positionx - us[select].positionx) <= sqr(us[select].attackRange);
	}
	bool buttonPressed = contMouse(battlefield[they[i].positiony][they[i].positionx]);
	return inRange && buttonPressed;
}
bool moveCondition(vector<vector<RectangleShape>>& battlefield, int i, int j) {
	return contMouse(battlefield[i][j]) && pythagor(i - us[select].positiony, j - us[select].positionx) <= sqr(us[select].ms) && battlefieldState[i][j] == "free";
}
bool activeCondition() {
	return us[select].hasActive && contMouse(us[select].activeButton) && us[select].cooldown == 0 && us[select].fireballNumber > 0;
}
bool altActiveCondition() {
	return us[select].hasActive && contMouse(us[select].altActiveButton) && us[select].altCooldown == 0 && us[select].healNumber > 0;
}
bool weCanInteract(Clock& rattle) {
	return globalState == PGS::free && select != -1 && rattle.getElapsedTime().asMilliseconds() >= 500 && Mouse::isButtonPressed(Mouse::Left);
}
void usChoosePositionToAttack(int& targetx, int& targety, int i) {
	if (they[i].positionx > us[select].positionx) {
		targetx--;
	}
	else if (they[i].positionx < us[select].positionx) {
		targetx++;
	}
	else if (us[select].positiony < they[i].positiony) {
		targety--;
	}
	else if (us[select].positiony > they[i].positiony) {
		targety++;
	}
}
void paintDefaultBattlefield(vector<vector<RectangleShape>>& battlefield) {
	for (int inow = 0; inow < h; inow++) {
		for (int jnow = 0; jnow < w; jnow++) {
			battlefield[inow][jnow].setFillColor(Color::Transparent);
		}
	}
}
bool skipTurnCondition(Sprite& skipTurnButton) {
	return !us[select].skippedTurnThisRound && contMouse(skipTurnButton);
}
void usPossibleActions(Clock& rattle, vector<vector<RectangleShape>>& battlefield, Sprite& skipTurnButton, Sprite& concedeButton) {
	if (skipTurnCondition(skipTurnButton)) {
		us[select].skippedTurnThisRound = true;
		select = -1;
		rattle.restart();
		return;
	}
	if (contMouse(concedeButton)) {
		globalState = PGS::conceding;
		rattle.restart();
	}
	for (int i = 0; i < they.size(); i++) {
		if (usAttackCondition(i, battlefield)) {
			int targetx = they[i].positionx - us[select].positionx;
			int targety = they[i].positiony - us[select].positiony;
			if (!us[select].isRangeUnit) {
				battlefieldState[us[select].positiony][us[select].positionx] = "free";
				usChoosePositionToAttack(targetx, targety, i);
				battlefieldState[us[select].positiony + targety][us[select].positionx + targetx] = "friend";
			}
			us[select].attack(targetx, targety);
			damaged = i;
			rattle.restart();
			return;
		}
	}
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (moveCondition(battlefield, i, j)) {
				battlefieldState[us[select].positiony][us[select].positionx] = "free";
				us[select].move(j - us[select].positionx, i - us[select].positiony);
				battlefieldState[i][j] = "friend";
				rattle.restart();
				return;
			}
		}
	}
	if (activeCondition()) {
		us[select].useActive();
		paintDefaultBattlefield(battlefield);
		us[select].cooldown = us[select].defaultActiveCooldown;
		us[select].fireballNumber--;
		rattle.restart();
		return;
	}
	if (altActiveCondition()) {
		us[select].useAltActive();
		paintDefaultBattlefield(battlefield);
		us[select].altCooldown = us[select].defaultAltActiveCooldown;
		us[select].healNumber--;
		rattle.restart();
		return;
	}
}
bool continueAttackAnimationCondition(Clock& rattle) {
	return damaged != -1 && select != -1 && globalState == PGS::damage && rattle.getElapsedTime().asMilliseconds() >= 10;
}
bool continueMoveAnimationCondition(Clock& rattle) {
	return select != -1 && globalState == PGS::animation && rattle.getElapsedTime().asMilliseconds() >= 10;
}
bool cancelAbilityCondition(Clock& rattle) {
	return select != -1 && us[select].hasActive && contMouse(us[select].activeButton) && Mouse::isButtonPressed(Mouse::Left) && globalState == PGS::active_ability && rattle.getElapsedTime().asMilliseconds() >= 500;
}
bool continueParticleAnimationCondition(Clock& rattle) {
	return globalState == PGS::particle_flying && rattle.getElapsedTime().asMilliseconds() >= 10 && damaged == -1;
}
bool activatingActiveCondition() {
	return globalState == PGS::activating_active && nowtargetx != -1 && nowtargety != -1;
}
bool activatingAltActiveCondition() {
	return globalState == PGS::activating_Altactive && nowtargetx != -1 && nowtargety != -1;
}
void paintActiveSight(vector<vector<RectangleShape>>& battlefield, int i, int j) {
	battlefield[i][j].setFillColor(Color(255, 0, 0, 100));
	int minx = max(0, j - actives[us[select].active].area);
	int miny = max(0, i - actives[us[select].active].area);
	int maxx = min(10, j + actives[us[select].active].area);
	int maxy = min(5, i + actives[us[select].active].area);
	for (int inow = 0; inow < h; inow++) {
		for (int jnow = 0; jnow < w; jnow++) {
			if (inow<miny || inow>maxy || jnow<minx || jnow>maxx) {
				battlefield[inow][jnow].setFillColor(Color::Transparent);
			}
			else {
				battlefield[inow][jnow].setFillColor(Color(255, 0, 0, 100));
			}
		}
	}
}
bool legalActiveTarget(vector<vector<RectangleShape>>& battlefield, Clock& rattle, int i, int j) {
	bool goodTime = rattle.getElapsedTime().asMilliseconds() >= 500 && globalState == PGS::active_ability && select != -1;
	if (!goodTime) {
		return false;
	}
	bool goodPosition = contMouse(battlefield[i][j]) && pythagor(us[select].positionx - j, us[select].positiony - i) <= sqr(actives[us[select].active].range);
	return goodTime && goodPosition && Mouse::isButtonPressed(Mouse::Left);
}
void paintBattlefieldWithMoveRange(vector<vector<RectangleShape>>& battlefield, int i, int j) {
	if (pythagor(i - us[select].positiony, j - us[select].positionx) <= sqr(us[select].ms)) {
		battlefield[i][j].setFillColor(Color(0, 255, 0, 100));
	}
	else if (!us[select].isRangeUnit && battlefieldState[i][j] == "enemy" && pythagor(i - us[select].positiony, j - us[select].positionx) <= sqr(us[select].ms) + 1) {
		battlefield[i][j].setFillColor(Color(0, 255, 0, 100));
	}
	else if (us[select].isRangeUnit && battlefieldState[i][j] == "enemy" && pythagor(i - us[select].positiony, j - us[select].positionx) <= sqr(us[select].attackRange)) {
		battlefield[i][j].setFillColor(Color(0, 255, 0, 100));
	}
	else {
		battlefield[i][j].setFillColor(Color::Transparent);
	}
}
void colouriseBattlefieldUs(vector<vector<RectangleShape>>& battlefield) {
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (select != -1 && globalState == PGS::free) {
				paintBattlefieldWithMoveRange(battlefield, i, j);
			}
			else {
				if (globalState == PGS::active_ability && select != -1) {
					if (contMouse(battlefield[i][j]) && pythagor(us[select].positionx - j, us[select].positiony - i) <= sqr(actives[us[select].active].range)) {
						paintActiveSight(battlefield, i, j);
					}
					else if (contMouse(battlefield[i][j]) && pythagor(us[select].positionx - j, us[select].positiony - i) > sqr(actives[us[select].active].range)) {
						paintDefaultBattlefield(battlefield);
					}
				}
			}
		}
	}
}
bool aiNeedToChoose() {
	return globalState == PGS::free && select != -1;
}
bool aiCanUseActive() {
	return they[select].hasActive && they[select].cooldown == 0;
}
void makeAnswer() {
	for (int i = 0; i < 2; i++) {
		if (us[i].alive) {
			answer[i] = us[i].hp;
		} else {
			answer[i] = 0;
		}
	}
	answer[2] = us[0].healNumber;
	answer[3] = us[0].fireballNumber;
}
bool somebodyWin(RenderWindow& window) {
	if (concede) {
		makeAnswer();
		answer.push_back(0);
		//cout << "You just run from your problems....";
		return true;
	}
	bool allEnemyAreDead = true;
	bool allFriendsAreDead = true;
	for (size_t i = 0; i < us.size(); i++) {
		if (us[i].alive) {
			allFriendsAreDead = false;
			break;
		}
	}
	for (size_t i = 0; i < they.size(); i++) {
		if (they[i].alive) {
			allEnemyAreDead = false;
			break;
		}
	}
	if (allEnemyAreDead && allFriendsAreDead) {
		makeAnswer();
		answer.push_back(2);
		//cout << "FRIENDSHIP WON!!!!1!";
		return true;
	}
	if (allEnemyAreDead) {
		makeAnswer();
		answer.push_back(1);
		//cout << "U WIN!!!!11!1!!!!";
		return true;
	}
	if (allFriendsAreDead) {
		makeAnswer();
		answer.push_back(0);
		//cout << "Unluchka, unluchka((((";
		return true;
	}
	return false;
}
void findEarthActiveTarget(int& targetx, int& targety) {
	int betterValue = 0;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (pythagor(j - they[select].positionx, i - they[select].positiony) <= sqr(actives[they[select].active].range)) {
				int countValue = 0;
				for (int y = max(0, i - actives[they[select].active].area); y < min(5, i + actives[they[select].active].area); y++) {
					for (int x = max(0, j - actives[they[select].active].area); x < min(10, j + actives[they[select].active].area); x++) {
						if (battlefieldState[y][x] == "friend") {
							countValue++;
						}
						else if (battlefieldState[y][x] == "enemy") {
							countValue--;
						}
					}
				}
				if (countValue > betterValue) {
					targetx = j;
					targety = i;
				}
			}
		}
	}
}
bool hasTarget(int targetx, int targety) {
	return targetx != -1 && targety != -1;
}
void aiFindTargetToAttack(int& targetx, int& targety, int i) {
	targetx = us[i].positionx - they[select].positionx;
	targety = us[i].positiony - they[select].positiony;
	if (they[select].isRangeUnit) {
		return;
	}
	if (us[i].positionx > they[select].positionx) {
		targetx--;
	}
	else if (us[i].positionx < they[select].positionx) {
		targetx++;
	}
	else if (they[select].positiony < us[i].positiony) {
		targety--;
	}
	else if (they[select].positiony > us[i].positiony) {
		targety++;
	}
}
bool aiCanAttackThis(int i) {
	if (!they[select].isRangeUnit) {
		return us[i].alive && pythagor(us[i].positionx - they[select].positionx, us[i].positiony - they[select].positiony) <= sqr(they[select].ms);
	}
	return us[i].alive && pythagor(us[i].positionx - they[select].positionx, us[i].positiony - they[select].positiony) <= sqr(they[select].attackRange);
}
bool aiDestinationBusy(int dx, int dy) {
	return battlefieldState[they[select].positiony + dy][they[select].positionx + dx] != "free";
}
void aiFindTargetToMove(int& dx, int& dy) {
	int minrastkv = -1;
	int tomove = -1;
	for (size_t i = 0; i < us.size(); i++) {
		int rastnow = pythagor(us[i].positionx - they[select].positionx, us[i].positiony - they[select].positiony);
		if ((rastnow < minrastkv || minrastkv == -1) && us[i].alive) {
			minrastkv = rastnow;
			tomove = i;
		}
		else if (us[i].alive && rastnow == minrastkv && us[tomove].dmg < us[i].dmg) {
			tomove = i;
		}
	}
	nowtargetx = us[tomove].positionx;
	nowtargety = us[tomove].positiony;
	int kraz = sqrt(minrastkv / (they[select].ms * they[select].ms));
	dx = (nowtargetx - they[select].positionx) / kraz;
	dy = (nowtargety - they[select].positiony) / kraz;
	if (pythagor(dx, dy) > sqr(they[select].ms)) {
		if (dx >= dy) {
			dx--;
		}
		else {
			dy--;
		}
	}
}
void aiActions() {
	if (aiCanUseActive()) {
		if (actives[they[select].active].target == "earth") {
			int targetx = -1;
			int targety = -1;
			findEarthActiveTarget(targetx, targety);
			if (hasTarget(targetx, targety)) {
				actives[they[select].active].activate(targetx, targety, they);
				return;
			}
		}
	}
	for (size_t i = 0; i < us.size(); i++) {
		if (aiCanAttackThis(i)) {
			int targetx, targety;
			aiFindTargetToAttack(targetx, targety, i);
			if (!they[select].isRangeUnit) {
				battlefieldState[they[select].positiony][they[select].positionx] = "free";
				battlefieldState[they[select].positiony + targety][they[select].positionx + targetx] = "enemy";
			}
			they[select].attack(targetx, targety);
			damaged = i;
			return;
		}
	}
	int dx, dy;
	aiFindTargetToMove(dx, dy);
	if (aiDestinationBusy(dx, dy)) {
		int stepx = (they[select].positionx - nowtargetx) / abs(they[select].positionx - nowtargetx);
		int stepy = (they[select].positiony - nowtargety) / abs(they[select].positiony - nowtargety);
		int i = 0;
		while (aiDestinationBusy(dx, dy)) {
			if (i % 2 == 0) {
				dx += stepx;
			}
			else {
				dy += stepy;
			}
			i++;
		}
	}
	battlefieldState[they[select].positiony][they[select].positionx] = "free";
	battlefieldState[they[select].positiony + dy][they[select].positionx + dx] = "enemy";
	they[select].move(dx, dy);
	nowtargetx = -1;
	nowtargety = -1;
	return;
}
bool continueBulletAnimationCondition(Clock& rattle) {
	return damaged != -1 && (globalState == PGS::particle_flying || globalState == PGS::dealing_damage) && rattle.getElapsedTime().asMilliseconds() >= 10; //mb select
}
void botTurn(Clock& rattle, vector<vector<RectangleShape>>& battlefield) {
	if (!they[select].alive) {
		select = -1;
	}
	if (aiNeedToChoose()) {
		aiActions();
	}
	if (continueMoveAnimationCondition(rattle)) {
		they[select].move();
		rattle.restart();
	}
	if (continueBulletAnimationCondition(rattle)) {
		they[select].attack(us[damaged], they, us);
		rattle.restart();
	}
	if (continueParticleAnimationCondition(rattle)) {
		actives[they[select].active].txt.draw(true);
		rattle.restart();
	}
	if (activatingActiveCondition()) {
		actives[they[select].active].activate(us[select]);
	}
	if (continueAttackAnimationCondition(rattle)) {
		they[select].attack(us[damaged], they, us);
		rattle.restart();
	}
	paintDefaultBattlefield(battlefield);
}
bool playerChooseFaith(Clock& rattle) {
	return Mouse::isButtonPressed(Mouse::Left) && rattle.getElapsedTime().asMilliseconds() > 500 && globalState == PGS::conceding;
}
bool playerConcede(Clock& rattle) {
	rattle.restart();
	return crossroads.playerRun();
}
void playerTurn(Clock& rattle, vector<vector<RectangleShape>>& battlefield, Sprite& skipTurnButton, Sprite& concedeButton) {
	if (!us[select].alive) {
		select = -1;
	}
	if (playerChooseFaith(rattle)) {
		if (playerConcede(rattle)) {
			concede = true;
		}
		else {
			globalState = PGS::free;
		}
	}
	if (weCanInteract(rattle)) {
		usPossibleActions(rattle, battlefield, skipTurnButton, concedeButton);
	}
	if (continueAttackAnimationCondition(rattle)) {
		us[select].attack(they[damaged], us, they);
		rattle.restart();
	}
	if (continueMoveAnimationCondition(rattle)) {
		us[select].move();
		rattle.restart();
	}
	if (cancelAbilityCondition(rattle)) {
		globalState = PGS::free;
		rattle.restart();
		us[select].cooldown = 0;
		us[select].fireballNumber++;
	}
	if (continueParticleAnimationCondition(rattle)) {
		actives[us[select].active].txt.draw(true);
		rattle.restart();
	}
	if (continueBulletAnimationCondition(rattle)) {
		us[select].attack(they[damaged], us, they);
		rattle.restart();
	}
	if (activatingActiveCondition()) {
		actives[us[select].active].activate(us[select]);
	}
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (legalActiveTarget(battlefield, rattle, i, j)) {
				actives[us[select].active].activate(j, i, us);
				rattle.restart();
			}
		}
	}
	if (globalState == PGS::Altactive_ability) {
		actives[us[select].altActive].activate(us[select]);
		rattle.restart();
	}
	colouriseBattlefieldUs(battlefield);
}
void giveStats(Unit& target, string& inf, string type = "", int flu = 0, int bmb =0) {
	if (inf == "we") {
		target.dmg = 5;
		target.hpMax = 200;
		target.setType(inf);
		target.ms = 3;
		target.hasActive = true;
		target.active = 0;
		target.altActive = 1;
		target.defaultActiveCooldown = 3;
		target.initiative = 10;
		target.statuses = statuses;
		target.fireballNumber = bmb;
		target.healNumber = flu;
		target.altCooldown = 0;
		target.defaultAltActiveCooldown = 1;
	}
	else if (inf == "dog") {
		target.dmg = 2;
		target.hpMax = 7;
		target.setType(inf);
		target.hasActive = false;
		target.ms = 5;
		target.initiative = 1000;
		target.statuses = statuses;
	} else if(type == "wizard") {
		target.ms = 3;
		target.initiative = 1;
		target.dmg = 1;
		target.hpMax = 6;
		target.hp = 6;
		target.isRangeUnit = true;
		target.setType(inf);
		target.statuses = statuses;
		target.hasActive = true;
		target.active = 0;
		target.defaultActiveCooldown = 3;
		target.isRangeUnit = true;
		target.attackRange = 3;
		Particle bul;
		bul.txt = RectangleShape();
		bul.txt.setFillColor(Color::Red);
		bul.startSize = 20;
		bul.endSize = 20;
		bul.deltaX = 0;
		bul.deltaY = 0;
		target.altActive = -1;
		target.bullet = bul;
		target.fireballNumber = 5;
		target.healNumber = 0;
	}
	else if (type == "warrior") {
		target.dmg = 3;
		target.hpMax = 8;
		target.hp = 6;
		target.setType(inf);
		target.ms = 4;
		target.initiative = 5;
		target.statuses = statuses;
	} else if (type == "archer") {
		target.ms = 3;
		target.initiative = 2;
		target.dmg = 3;
		target.hpMax = 6;
		target.hp = 6;
		target.isRangeUnit = true;
		target.setType(inf);
		target.statuses = statuses;
		target.isRangeUnit = true;
		target.attackRange = 5;
		Particle bul;
		bul.txt = RectangleShape();
		bul.txt.setFillColor(Color::Black);
		bul.startSize = 20;
		bul.endSize = 20;
		bul.deltaX = 0;
		bul.deltaY = 0;
		target.bullet = bul;
	}
	
}
void coloriseAsistance(vector<vector<RectangleShape>>& battlefield) {
	int us_count = 0;
	int they_count = 0;
	for (size_t i = 0; i < turnTrackerWindows.size(); i++) {
		if (trackerFriends[i]) {
			if (contMouse(turnTrackerWindows[i]) && us[us_count].alive) {
				battlefield[us[us_count].positiony][us[us_count].positionx].setFillColor(Color(255, 0, 0, 100));
			}
			if (!nowBotTurn && us_count == select) {
				turnTrackerWindows[i].setFillColor(Color::Red);
			} else {
				turnTrackerWindows[i].setFillColor(Color::Blue);
			}
			us_count++;
		} else {
			if (contMouse(turnTrackerWindows[i]) && they[they_count].alive) {
				battlefield[they[they_count].positiony][they[they_count].positionx].setFillColor(Color(255, 0, 0, 100));
			}
			if (nowBotTurn && they_count == select) {
				turnTrackerWindows[i].setFillColor(Color::Red);
			}
			else {
				turnTrackerWindows[i].setFillColor(Color::Blue);
			}
			they_count++;
		}
	}
}
void buildTurnTracker(int pointer_us, int pointer_they) {
	while (pointer_us < us.size() || pointer_they < they.size()) {
		turnTracker.push_back(Sprite());
		turnTrackerWindows.push_back(RectangleShape());
		turnTrackerWindows.back().setOutlineThickness(5);
		turnTrackerWindows.back().setOutlineColor(Color::Blue);
		turnTrackerWindows.back().setFillColor(Color::Blue);
		turnTrackerWindows.back().setSize(Vector2f(75.f, 75.f));
		chooseSelect(pointer_us, pointer_they);
		if (nowBotTurn) {
			turnTracker.back().setTexture(they[select].txtt);
			trackerFriends.push_back(false);
		}
		else {
			turnTracker.back().setTexture(us[select].txtt);
			trackerFriends.push_back(true);
		}
		select = -1;
		turnTracker.back().setScale(0.5, 0.5);
	}
	for (size_t i = 0; i < turnTracker.size();i++) {
		int minBreak = (1920 - (75 * int(turnTracker.size()))) / 2 - 40;
		Vector2f pos(minBreak + i * 90, 950.f);
		turnTracker[i].setPosition(pos);
		turnTrackerWindows[i].setPosition(pos);
	}
}
//возвращает: {хп игрока, хп собаки, осталось фласок, осталось бомб, 1 если вин 0 если луз 2 если ничья}
vector<int> letTheBattleBegin(vector<pair<string, string>> infAboutEnemys, int ushp, int doghp, int flusksm, int bombs){	
	//vector<pair<string, string>> infAboutEnemys = {{"enemyWizard", "wizard"},{"enemyWarrior", "warrior"}, {"enemyArcher", "archer"} };//first = name, second = type
	//int ushp = 200;
	//int doghp = 2;
	//int flusks = 5;
	//int bombs = 5;

	int usposx = 0, usposy = 1, dogposx = 0, dogposy = h-2;



	int enemyNumber = int(infAboutEnemys.size());
	int breakYForUnit = h / (enemyNumber + 1);
	int nowYForUnit = breakYForUnit;

	textures["background"] = Texture();
	textures["we"] = Texture();
	textures["healButton"] = Texture();
	textures["fireballButton"] = Texture();
	textures["dog"] = Texture();
	textures["skipTurn"] = Texture();
	textures["active"] = Texture();
	textures["concedeButton"] = Texture();
	textures["yesButton"] = Texture();
	textures["noButton"] = Texture();
	
	if (!textures["background"].loadFromFile("imgs/background.jpg") || !textures["we"].loadFromFile("imgs/we.png") || !textures["healButton"].loadFromFile("imgs/healButton.png")) {
		return vector<int>(0);
	}
	if (!textures["fireballButton"].loadFromFile("imgs/fireballButton.png") || !textures["concedeButton"].loadFromFile("imgs/concedeButton.png")) {
		return vector<int>(0);
	}
	if (!textures["active"].loadFromFile("imgs/activeTexture.png") || !textures["skipTurn"].loadFromFile("imgs/skipTurnTexture.png") || !textures["dog"].loadFromFile("imgs/dog.png")) {
		return vector<int>(0);
	}
	if (!textures["yesButton"].loadFromFile("imgs/yesButton.png") || !textures["noButton"].loadFromFile("imgs/noButton.png")) {
		return vector<int>(0);
	}

	Font main_font;
	if (!main_font.loadFromFile("mainFont.ttf")) return vector<int>(0);

	crossroads = ConcedeMenu(main_font);

	ActiveAbility heal;
	heal.name = "Heal";
	ActiveAbility fireball;
	fireball.name = "Fireball";
	actives.push_back(fireball);
	actives[0].range = 5;
	actives[0].addedStatus = {};
	actives[0].removedStatus = {};
	actives[0].target = "earth";
	actives[0].hpChange = 20;
	actives[0].area = 1;
	actives[0].id = 0;
	actives.push_back(heal);
	actives[1].id = 1;
	actives[1].addedStatus = {};
	actives[1].removedStatus = {};
	actives[1].target = "self";
	actives[1].hpChange = -5;

	for (size_t i = 0; i < actives.size(); i++) {
		if (actives[i].target == "earth") {
			if (!textures[actives[i].name].loadFromFile("imgs/" + actives[i].name + ".png")) {
				return vector<int>(0);
			}
		}
	}
	for (size_t i = 0; i < infAboutEnemys.size(); i++) {
		if (!textures[infAboutEnemys[i].first].loadFromFile("imgs/" + infAboutEnemys[i].first + ".png")) {
			return vector<int>(0);
		}
	}

	

	StatsTable table(main_font);

	Sprite skipTurnButton;
	skipTurnButton.setTexture(textures["skipTurn"]);
	skipTurnButton.setPosition(Vector2f(7.f, 925.f));
	Sprite concedeButton;
	concedeButton.setTexture(textures["concedeButton"]);
	concedeButton.setPosition(Vector2f(162.f, 925.f));
	
	Particle ballOfFire;
	ballOfFire.deltaX = -100;
	ballOfFire.deltaY = -150;
	ballOfFire.startSize = 1;
	ballOfFire.endSize = 450;
	ballOfFire.txt = RectangleShape();
	ballOfFire.txt.setTexture(&textures["Fireball"]);
	
	actives[0].txt = ballOfFire;
	
	StatBlock URPOISONED;
	URPOISONED.tickdmg = 3;
	Passive poisoned;
	poisoned.type = "status";
	poisoned.id = 0;
	poisoned.dmg = 0;
	poisoned.time = 3;
	poisoned.friendStats = URPOISONED;
	Passive poison_touch;
	poison_touch.name = "Poison Touch";
	poison_touch.id = 1;
	poison_touch.trigger = "attack";
	poison_touch.target = "attacked";
	poison_touch.effects_enemy.push_back(poisoned);
	poison_touch.type = "ability";
	Passive running;
	running.id = 2;
	running.type = "status";
	running.time = 1;
	StatBlock runningStats;
	runningStats.msChange = 2;
	running.friendStats = runningStats;
	runningStats.tickdmg = 0;
	Passive runrunrun; ;
	runrunrun.name = "Coward";
	runrunrun.condition = isEnemyNear;
	runrunrun.id = 3;
	runrunrun.effects_friends.push_back(running);
	runrunrun.trigger = "turn start";
	runrunrun.type = "ability";
	runrunrun.target = "self";
	runrunrun.time = 1;
	statuses.push_back(poisoned);
	statuses.push_back(poison_touch);
	statuses.push_back(running);
	statuses.push_back(runrunrun);


	Unit scytheofvyse;
	for (size_t i = 0; i < infAboutEnemys.size(); i++) {
		while (nowYForUnit >= h) {
			nowYForUnit--;
		}
		scytheofvyse = Unit(10, nowYForUnit);
		giveStats(scytheofvyse, infAboutEnemys[i].first, infAboutEnemys[i].second);
		scytheofvyse.statuses = statuses;
		they.push_back(scytheofvyse);
		nowYForUnit += breakYForUnit;
	}

	//srand(time(nullptr));//задаём сид для генерации случайных чисел (что это и почему это желательно сделать можно почитать на cppreferences)

	RenderWindow window(VideoMode(1920, 1080), L"Охота на Ликантропа: драка", Style::Default);
	window.setVerticalSyncEnabled(true);
	Clock rattle;
	Sprite bg;
	bg.setTexture(textures["background"]);
	bg.setScale(1.43, 1.43);

	vector<vector<RectangleShape>> battlefield(h, vector<RectangleShape>(w));
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			RectangleShape hex(Vector2f(150.f, 150.f));
			hex.setOutlineColor(Color::Blue);
			hex.setOutlineThickness(2.f);
			hex.setFillColor(Color::Transparent);
			hex.move(j * 150.f + 135.f, i * 150.f + 5);
			battlefield[i][j] = hex;
		}
	}

	scytheofvyse = Unit(dogposx,dogposy);
	scytheofvyse.hp = doghp;
	string mysticStaff = "dog";
	giveStats(scytheofvyse, mysticStaff);
	scytheofvyse.passives_whenAttack.push_back(poison_touch);
	scytheofvyse.passives_whenAttack.push_back(runrunrun);
	scytheofvyse.statuses = statuses;
	us.push_back(scytheofvyse);

	scytheofvyse = Unit(usposx, usposy);
	scytheofvyse.hp = ushp;
	mysticStaff = "we";
	giveStats(scytheofvyse, mysticStaff, "", 5, 5);
	scytheofvyse.statuses = statuses;
	us.push_back(scytheofvyse);

	sort(us.begin(), us.end());
	sort(they.begin(), they.end());
	int pointer_they = 0;
	int pointer_us = 0;

	buildTurnTracker(pointer_us, pointer_they);

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}
		if (somebodyWin(window)) {
			return answer;
			break;
		}
		if (select == -1) {
			while (select == -1) {
				chooseSelect(pointer_us, pointer_they);
			}
			activateTurnStartOfSelected();
		}
		if (nowBotTurn) {
			botTurn(rattle, battlefield);
		}
		else {
			playerTurn(rattle, battlefield, skipTurnButton, concedeButton);

			if (select != -1 && us[select].skippedTurnThisRound) {
				skipTurnButton.setColor(Color(0, 0, 0, 255));
			}
			else if (select != -1 && !us[select].skippedTurnThisRound) {
				skipTurnButton.setColor(Color(255, 255, 255, 255));
			}
		}
		window.draw(bg);
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				coloriseAsistance(battlefield);
				window.draw(battlefield[i][j]);
			}
		}
		for (int i = 0; i < us.size(); i++) {
			us[i].draw(window);
		}
		for (int i = 0; i < they.size(); i++) {
			they[i].draw(window);
		}
		if (globalState == PGS::particle_flying && damaged == -1) {
			window.draw(actives[us[select].active].txt.txt);
		}
		if (globalState == PGS::particle_flying && damaged != -1) {
			if (!nowBotTurn) {
				window.draw(us[select].bullet.txt);
			}
			else {
				window.draw(they[select].bullet.txt);
			}
		}
		for (size_t i = 0; i < turnTracker.size(); i++) {
			window.draw(turnTrackerWindows[i]);
			window.draw(turnTracker[i]);
		}
		window.draw(skipTurnButton);
		for (size_t i = 0; i < us.size(); i++) {
			if (us[i].alive && contMouse(us[i].txt) && Mouse::isButtonPressed(Mouse::Right)) {
				table.draw(us[i], window);
			}
		}
		for (size_t i = 0; i < they.size(); i++) {
			if (they[i].alive && contMouse(they[i].txt) && Mouse::isButtonPressed(Mouse::Right)) {
				table.draw(they[i], window);
			}
		}
		window.draw(concedeButton);
		if (globalState == PGS::conceding) {
			crossroads.draw(window);
		}
		window.display();
	}
	return vector<int>(0);
}
int main() {
	letTheBattleBegin({ {"enemyWizard", "wizard"},{"enemyWarrior", "warrior"}, {"enemyArcher", "archer"} }, 200, 2, 5, 5);
	//(vector<pair<string, string>> infAboutEnemys, int ushp, int doghp, int flusksm, int bombs)
}
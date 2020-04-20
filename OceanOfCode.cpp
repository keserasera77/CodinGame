#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <chrono>

using namespace std;

#define rep(i,n) for(int i = 0; i < (n); ++i)
#define rrep(i,n) for(int i = n-1; i >= 0; --i)
#define rep1(i,n) for(int i = 1; i <= (n); ++i)
#define rrep1(i,n) for(int i = (n); i >= 1; --i)
#define all(vec) (vec).begin(),(vec).end() 

#define debug(vec) for(auto v : vec) cerr << v << " "; cerr << endl;
#define debugP(vec) for(auto v : vec) cerr << "(" << v.first << "," << v.second << ") "; cerr << endl;
#define debug2D(vec2D)  rep(y,height) { rep(x, width) cerr << vec2D[x][y] << " "; cerr << endl; } 


typedef vector<vector<bool>> fieldB;
#define defField1(name) fieldB name(width, vector<bool> (height));
#define defField2(name, b) fieldB name(width, vector<bool> (height, b));

template<class T>inline bool chmax(T& a, T b) { if (a < b) { a = b; return true; } else return false; }
template<class T>inline bool chmin(T& a, T b) { if (a > b) { a = b; return true; } else return false; }

constexpr int EARLY_TURN = 30;
constexpr int FIRE_BORDER = 3; //正方形の1辺の長さ
constexpr int SILENCE_MYPOSCNT = 10;
constexpr int SILENCE_MOVABLECNT = 5;
constexpr int SONAR_BORDER = 50;
constexpr int SONAR_START_TIME = 30;
constexpr int LARGE_AREA = 50;

void myAssert(bool cond, string s) {
	if (!cond) cerr << "MyAssert!! : " << s << endl;
}

void writeTime(chrono::system_clock::time_point start) {
	chrono::system_clock::time_point end = chrono::system_clock::now();
	cerr << static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0) << "[ms]" << endl;
}

template<class T>
T safeAt(vector<vector<T>> vec2, int _x, int _y) {
	int x = _x, y = _y;
	if (x < 0) x = 0; if (y < 0) y = 0;
	if (x >= vec2.size()) x = vec2.size() - 1; if (y >= vec2[0].size()) y = vec2[0].size() - 1;
	return vec2[x][y];
}

enum Dir {
	N,
	E,
	S,
	W
};

enum State {
	Early,
	Safe,
	Danger,
};

const int dirX[4] = { 0,1,0,-1 }, dirY[4] = { -1,0,1,0 };

char dtoc(Dir dir) {
	if (dir == N) return 'N';
	if (dir == E) return 'E';
	if (dir == S) return 'S';
	if (dir == W) return 'W';
	myAssert(1, "dtoc : 存在しない方角 , dir = " + to_string(dir));
}

Dir itod(int i) {
	switch (i) {
	case 0: return N; break;
	case 1: return E; break;
	case 2: return S; break;
	case 3: return W; break;
	default: myAssert(false, "itod : OUT OF RANGE i = " + to_string(i)); break;
	}
}

bool isInField(int height, int width, int x, int y) {
	return 0 <= x && x < width && 0 <= y && y < height;
}

bool isInWater(const fieldB& isWater, int x, int y) {
	return isInField(isWater[0].size(), isWater.size(), x, y) && isWater[x][y];
}

bool canMove(const fieldB& isWater, const fieldB& gone, int x, int y) {
	return isInWater(isWater, x, y) && !gone[x][y];
}

int _countMovable(const fieldB& isWater, const fieldB& gone, int sx, int sy) {
	int height = isWater[0].size(), width = isWater.size(), ret = 0;
	defField2(done, false);
	queue<pair<int, int>> que;
	que.push({ sx,sy });
	while (!que.empty()) {
		pair<int, int> v = que.front();
		que.pop();
		int x = v.first, y = v.second;

		if (!canMove(isWater, gone, x, y) || done[x][y]) continue;
		done[x][y] = true;

		ret++;
		que.push({ x + 1,y });
		que.push({ x - 1,y });
		que.push({ x,y - 1 });
		que.push({ x,y + 1 });
	}

	return ret;
}

int countMovable(const fieldB& isWater, fieldB gone, int sx, int sy) {
	if (!canMove(isWater, gone, sx, sy)) return 0;
	gone[sx][sy] = true;
	int mx = 0;
	rep(dir, 4) {
		int c = _countMovable(isWater, gone, sx + dirX[dir], sy + dirY[dir]);
		if (c > 0) chmax(mx, c);
	}
	return mx;
}

bool canFire(int oppLife, int myLife, int myx, int myy, int fireX, int fireY) {
	int dX = abs(myx - fireX), dY = abs(myy - fireY);
	return !(dX == 0 && dY == 0 && (oppLife >= myLife || myLife <= 2))
		&& !(dX <= 1 && dY <= 1 && (oppLife >= myLife || myLife <= 1));
}

void torpedo(const fieldB& torpedoRange, vector<string>& orders, int x, int y) {
	if (isInField(torpedoRange[0].size(), torpedoRange.size(), x, y) && torpedoRange[x][y]) orders.push_back("TORPEDO " + to_string(x) + " " + to_string(y));
	else cerr << "OUT OF RANGE : x = " << x << ", y = " << y << endl;
}

void silence(vector<string>& orders, Dir dir, int dist) {
	string s = "SILENCE ";
	s += dtoc(dir); s += " "; s += to_string(dist);
	orders.push_back(s);
}

void move(vector<string>& orders, Dir dir, State myState, int torpedoCooldown, int sonarCooldown, int silenceCooldown, int mineCooldown) {
	string s = "MOVE ";
	s += dtoc(dir);
	string up = " TORPEDO";
	switch (myState) {
	case Early:
		if (mineCooldown > 0) up = " MINE";
		else if (torpedoCooldown > 0) up = " TORPEDO";
		else if (silenceCooldown > 0) up = " SILENCE";
		else if (sonarCooldown > 0) up = " SONAR";
		break;
	case Safe:
		if (torpedoCooldown > 0) up = " TORPEDO";
		else if (sonarCooldown > 0) up = " SONAR";
		else if (silenceCooldown > 0) up = " SILENCE";
		else if (mineCooldown > 0) up = " MINE";
		break;
	case Danger:
		if (torpedoCooldown > 0) up = " TORPEDO";
		else if (silenceCooldown > 0) up = " SILENCE";
		else if (sonarCooldown > 0) up = " SONAR";
		else if (mineCooldown > 0) up = " MINE";
		break;
	default:
		break;
	}
	s += up;
	orders.push_back(s);
}

void movePlan(Dir* dir, State myState, fieldB& oppPos, int height, int width, int centerX, int centerY, int myX, int myY) {
	switch (myState) {
	case Early:
		rrep(i, 4) rep(j, i) if (max(abs(centerX - myX - dirX[dir[j]]), abs(centerY - myY - dirY[dir[j]])) > max(abs(centerX - myX - dirX[dir[j + 1]]), abs(centerY - myY - dirY[dir[j + 1]]))) swap(dir[j], dir[j + 1]);
		break;
	case Safe:
		rrep(i, 4) rep(j, i) if (max(abs(centerX - myX - dirX[dir[j]]), abs(centerY - myY - dirY[dir[j]])) > max(abs(centerX - myX - dirX[dir[j + 1]]), abs(centerY - myY - dirY[dir[j + 1]]))) swap(dir[j], dir[j + 1]);
		break;
	case Danger:
		int cnt = 0, sumx = 0, sumy = 0;
		rep(x, width) rep(y, height) if (oppPos[x][y]) { sumx += x; sumy += y; cnt++; }
		myAssert(cnt != 0, "cnt == 0");
		int dx = sumx / cnt - myX, dy = sumy / cnt - myY;
		if (dy <= 0) swap(dir[S], dir[N]);
		if (dx >= 0) swap(dir[W], dir[E]);
		rep(i, 4) cerr << dir[i]; cerr << endl;
		break;
	}
}

void writeOrders(vector<string>& orders) {
	myAssert(!orders.empty(), "NO ORDER!!");
	rep(i, orders.size()) {
		cout << orders.at(i);
		if (i == orders.size() - 1) cout << endl;
		else cout << "|";
	}
}

void getOrders(vector<string>& oppOrders, string ords) {
	string ord;
	rep(i, ords.size()) {
		if (ords[i] == '|') {
			oppOrders.push_back(ord);
			ord.clear();
		}
		else ord += ords[i];
	}
	oppOrders.push_back(ord);
}

void setTorpedoRange(fieldB& fb, const fieldB& isWater, int sx, int sy) {
	queue<pair<pair<int, int>, int>> que;
	que.push({ {sx,sy},0 });
	while (!que.empty()) {
		pair<pair<int, int>, int> v = que.front();
		que.pop();
		int x = v.first.first, y = v.first.second, depth = v.second;

		if (depth > 4 || !isInWater(isWater, x, y) || fb[x][y]) continue;
		fb[x][y] = true;

		que.push({ {x + 1,y}, depth + 1 });
		que.push({ {x - 1,y}, depth + 1 });
		que.push({ {x,y + 1}, depth + 1 });
		que.push({ {x,y - 1}, depth + 1 });
	}
}

int posToSector(int height, int width, int x, int y) {
	int sector = 0;
	for (int yi = 0; yi < height; yi += 5) for (int xi = 0; xi < width; xi += 5) {
		sector++;
		if (yi <= y && y < yi + 5 && xi <= x && x < xi + 5) {
			return sector;
		}
	}

	return -1;
}

struct Gone {
	fieldB gone;
	vector<pair<int, int>> posFromSilence;
	int preSilenceTurn;
	vector<int> mineTurn;
	set<pair<int, int>> startPosFromSilence;

	int height;
	int width;

	Gone(int h, int w) : height(h), width(w), preSilenceTurn(0) {
		defField2(_gone, false);
		gone = _gone;
		posFromSilence.push_back({ 0,0 });
	}

	void surface(fieldB& pos, int turn) {
		rep(x, width) rep(y, height) gone[x][y] = false;
		posFromSilence.clear();
		posFromSilence.push_back({ 0,0 });
		preSilenceTurn = turn;
		startPosFromSilence.clear();
		rep(x, width) rep(y, height) {
			if (pos[x][y]) startPosFromSilence.insert({ x,y });
		}
	}

	void silence(fieldB& pos, int turn) {
		posFromSilence.clear();
		posFromSilence.push_back({ 0,0 });
		preSilenceTurn = turn;
		startPosFromSilence.clear();
		rep(x, width) rep(y, height) {
			if (pos[x][y]) startPosFromSilence.insert({ x,y });
		}
	}

	void mine(int turn) {
		mineTurn.push_back(turn);
	}

	bool startPosIsDetermined() { return startPosFromSilence.size() == 1; };

	void determineGone(int dx, int dy) {
		myAssert(startPosIsDetermined(), "determineGone : startPosIsDetermined() == false");
		if (startPosIsDetermined()) for (auto xy : posFromSilence) {
			//fast
			gone[xy.first + dx][xy.second + dy] = true;
		}
		cerr << "determine gone" << endl;
		debug2D(gone);
	}

	//相手のMINEが爆破したとき
	void inferGoneByMine(const fieldB& pos, const fieldB& isWater, int minex, int miney) {
		bool isFirstInfer = startPosFromSilence.empty();
		set<pair<int, int>> startPosFromSilence0, intersection;

		for (int turn : mineTurn) {
			if (turn < preSilenceTurn) continue;
			cerr << "turn = " << turn << " : x = " << posFromSilence[turn - preSilenceTurn - 1].first << " y = " << posFromSilence[turn - preSilenceTurn - 1].second << endl;
			rep(i, 4) {
				bool ok = true;
				int dx = minex + dirX[i] - posFromSilence[turn - preSilenceTurn - 1].first, dy = miney + dirY[i] - posFromSilence[turn - preSilenceTurn - 1].second;
				for (auto xy : posFromSilence) {
					//cerr << "(" << xy.first + dx << "," << xy.second + dy << ") ";
					if (!isInWater(isWater, xy.first + dx, xy.second + dy)) { ok = false; break; }
				}
				if (isInWater(isWater, posFromSilence.back().first + dx, posFromSilence.back().second + dy)) ok &= pos[posFromSilence.back().first + dx][posFromSilence.back().second + dy];
				if (ok) startPosFromSilence0.insert({ posFromSilence[0].first + dx, posFromSilence[0].second + dy });
			}
		}

		//毎inferGoneByMineでstartPosFromSilenceの集合の積を取っていく
		if (isFirstInfer) {
			startPosFromSilence = startPosFromSilence0;
			cerr << "startPosFromSilence" << endl;
			debugP(startPosFromSilence)
		}
		else {
			cerr << "startPosFromSilence" << endl;
			debugP(startPosFromSilence)
			cerr << "startPosFromSilence0" << endl;
			debugP(startPosFromSilence0)
			set<pair<int, int>> intersection;
			set<pair<int, int>>::iterator it = startPosFromSilence0.begin();
			for (auto xy : startPosFromSilence) {
				while (*it <= xy && it != startPosFromSilence0.end()) {
					if (*it == xy) intersection.insert(xy);
					++it;
				}
			}
			startPosFromSilence = intersection;
			cerr << "intersection" << endl;
			debugP(intersection);
		}

		if (startPosIsDetermined()) determineGone(startPosFromSilence.begin()->first, startPosFromSilence.begin()->second);
	}

	void inferGoneByPos(fieldB& pos) {
		int cnt = 0;
		vector<pair<int,int>> nowxy;
		rep(x, width) rep(y, height) {
			if (pos[x][y]) { cnt++; nowxy.push_back({x,y}); }
		}
		set<pair<int,int>> startPosFromSilence0;
		for(auto xy : nowxy) {
			int dx = xy.first - posFromSilence.back().first, dy = xy.second - posFromSilence.back().second;
			startPosFromSilence0.insert({ dx,dy });
		}
		set<pair<int, int>> intersection;
		set<pair<int, int>>::iterator it = startPosFromSilence0.begin();
		for (auto xy : startPosFromSilence) {
			while (*it <= xy && it != startPosFromSilence0.end()) {
				if (*it == xy) intersection.insert(xy);
				++it;
			}
		}
		startPosFromSilence = intersection;
		if (startPosIsDetermined()) determineGone(startPosFromSilence.begin()->first, startPosFromSilence.begin()->second);
	}
};

void inferPosByAttack(fieldB& oppPos , vector<pair<int, int>>& preMyAttackPos, int diffOppLife) {
  int height = oppPos[0].size(), width = oppPos.size();
	cerr << "preMyAttackPos : ";
	debugP(preMyAttackPos)

	cerr << diffOppLife << endl;
	//cerr << "pos" << endl;
	//debug2D(oppPos);

	auto isAround = [](int cX, int cY, int x, int y) {
	  return (abs(x - cX) <= 1 && abs(y - cY) <= 1) && !(x == cX && y == cY);
	};

	//前ターンの攻撃が当たったかどうか
	if (preMyAttackPos.size() == 1) {
	  int aX = preMyAttackPos[0].first, aY = preMyAttackPos[0].second;
		if (diffOppLife == 2) {
			rep(x, width) rep(y, height) {
				if (x == aX && y == aY) oppPos[x][y] = true;
				else oppPos[x][y] = false;
			}
		}
		else if (diffOppLife == 1) {
			rep(x, width) rep(y, height) {
				if (!isAround(aX,aY,x,y)) oppPos[x][y] = false;
				else cerr << "1 damage Zone : " << x << " " << y << endl;
			}
		}
		else /*diffOppLife == 0*/ {
		  myAssert(diffOppLife == 0, " inferPosByAttack : diffOppLife = " + to_string(diffOppLife));
			for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) if (isInField(oppPos[0].size(), oppPos.size(), aX + dx, aY + dy)) oppPos[aX + dx][aY + dy] = false;
		}
	}
	else if (preMyAttackPos.size() == 2) {
		int aX1 = preMyAttackPos[0].first, aY1 = preMyAttackPos[0].second;
		int aX2 = preMyAttackPos[1].first, aY2 = preMyAttackPos[1].second;

		if (diffOppLife == 4) {
			myAssert(aX1 == aX2 && aY1 == aY2, "inferPosByAttack : ??? " + to_string(aX1) + "," + to_string(aY1) + " " + to_string(aX2) + "," + to_string(aY2));
			rep(x, width) rep(y, height) {
				if (x == aX1 && y == aY1) oppPos[x][y] = true;
				else oppPos[x][y] = false;
			}
		}
		else if (diffOppLife == 3) {
			myAssert(abs(aX1 - aX2) <= 1 && abs(aY1 - aY2) <= 1, "inferPosByAttack : ??? " + to_string(aX1) + "," + to_string(aY1) + " " + to_string(aX2) + "," + to_string(aY2));
			rep(x, width) rep(y, height) {
				if (!(x == aX1 && y == aY1 || x == aX2 && y == aY2)) oppPos[x][y] = false;
			}
		}
		else if (diffOppLife == 2) {
			rep(x, width) rep(y, height) {
				if (!( isAround(aX1,aY1,x,y) && isAround(aX2,aY2,x,y) ) && !(x == aX1 && y == aY1) && !(x == aX2 && y == aY2)) oppPos[x][y] = false;
			}
		}
		else if (diffOppLife == 1) {
			rep(x, width) rep(y, height) {
				if (!( isAround(aX1, aY1, x, y) ^ isAround(aX2, aY2, x, y) )) oppPos[x][y] = false;
			}
		}
		else {
			myAssert(diffOppLife == 0, " inferPosByAttack : diffOppLife = " + to_string(diffOppLife));
			if (preMyAttackPos.empty()) {
				for(auto xy : preMyAttackPos) for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++)  if (isInField(oppPos[0].size(), oppPos.size(), xy.first + dx, xy.second + dy)) oppPos[xy.first + dx][xy.second + dy] = false;
			}
		}
	}

	preMyAttackPos.clear();
}

void inferPos(fieldB& oppPos, Gone& inferOppGone, const fieldB& isWater, const vector<string>& OppOrds, string mySonarResult, int mySonarResultSector, int turn, vector<pair<int,int>>& preMyAttackPos, vector<pair<int,int>>& preOppAttackPos, int diffOppLife) {
	inferPosByAttack(oppPos, preMyAttackPos, diffOppLife);
	int height = isWater[0].size(), width = isWater.size();
	//相手の行動の前の調査結果なのでこの位置でよい
	if (mySonarResult == "Y") {
		mySonarResultSector--;
		int ix = (mySonarResultSector % 3) * 5, iy = (mySonarResultSector / 3) * 5;
		rep(x, width) rep(y, height) {
			if (ix <= x && x < ix + 5 && iy <= y && y < iy + 5) continue;
			oppPos[x][y] = false;
		}
	}
	else if (mySonarResult == "N") {
		mySonarResultSector--;
		int ix = (mySonarResultSector % 3) * 5, iy = (mySonarResultSector / 3) * 5;
		for (int x = ix; x < ix + 5; x++) for (int y = iy; y < iy + 5; y++) oppPos[x][y] = false;
	}
	else myAssert(mySonarResult == "NA", "inferPos : invalidSonar " + mySonarResult); //TODO : inferPos : 相手の命令漏れ invalidSonarResult 1

	for (string ord : OppOrds) {
		vector<string> ss;
		string s;
		rep(i, ord.size()) {
			if (ord[i] == ' ' && !s.empty()) {
				ss.push_back(s);
				s.clear();
			}
			else s += ord[i];
		}
		if (!s.empty()) ss.push_back(s);

		if (ss[0] == "MOVE") {
			if (ss[1] == "S") {
				inferOppGone.posFromSilence.push_back({ inferOppGone.posFromSilence.back().first, inferOppGone.posFromSilence.back().second + 1 });
				rep(x, width) rrep(y, height - 1) oppPos[x][y + 1] = oppPos[x][y] && isWater[x][y + 1];
				rep(x, width) oppPos[x][0] = false;
			}
			else if (ss[1] == "W") {
				inferOppGone.posFromSilence.push_back({ inferOppGone.posFromSilence.back().first - 1, inferOppGone.posFromSilence.back().second });
				rep1(x, width - 1) rep(y, height) oppPos[x - 1][y] = oppPos[x][y] && isWater[x - 1][y];
				rep(y, height) oppPos[width - 1][y] = false;
			}
			else if (ss[1] == "N") {
				inferOppGone.posFromSilence.push_back({ inferOppGone.posFromSilence.back().first, inferOppGone.posFromSilence.back().second - 1 });
				rep(x, width) rep1(y, height - 1) oppPos[x][y - 1] = oppPos[x][y] && isWater[x][y - 1];
				rep(x, width) oppPos[x][height - 1] = false;
			}
			else if (ss[1] == "E") {
				inferOppGone.posFromSilence.push_back({ inferOppGone.posFromSilence.back().first + 1, inferOppGone.posFromSilence.back().second });
				rrep(x, width - 1) rep(y, height) oppPos[x + 1][y] = oppPos[x][y] && isWater[x + 1][y];
				rep(y, height) oppPos[0][y] = false;
			}
		}
		else if (ss[0] == "SURFACE") {
			myAssert(ss.size() == 2, "SURFACE is few");
			int sector = ss[1][0] - '1';
			int ix = (sector % 3) * 5, iy = (sector / 3) * 5;
			rep(x, width) rep(y, height) {
				if (ix <= x && x < ix + 5 && iy <= y && y < iy + 5) continue;
				oppPos[x][y] = false;
			}

			inferOppGone.surface(oppPos, turn);
		}
		else if (ss[0] == "TORPEDO") {
			defField2(tr, false);
			int _x = stoi(ss[1]), _y = stoi(ss[2]);
			setTorpedoRange(tr, isWater, _x, _y);
			//cerr << "tr : " << _x << " " << _y << endl;
			//debug2D(tr);
			rep(x, width) rep(y, height) oppPos[x][y] = oppPos[x][y] && tr[x][y];
			preOppAttackPos.push_back({_x,_y});
		}
		else if (ss[0] == "SONAR") {


		}
		else if (ss[0] == "SILENCE") {
			int xl = -5, xr = 5, yl = -5, yr = 5;
			pair<int, int> last = inferOppGone.posFromSilence.back();
			for (auto xy : inferOppGone.posFromSilence) {
				int x = last.first - xy.first, y = last.second - xy.second;
				if (x == 0) {
					if (y < 0) chmax(yl, y);
					else if (y > 0) chmin(yr, y);
				}
				else if (y == 0) {
					if (x < 0) chmax(xl, x);
					else if (x > 0) chmin(xr, x);
				}
			}
			cerr << "posFromSilence" << endl;
			debugP(inferOppGone.posFromSilence);
			cerr << xl << " " << xr << ", " << yl << " " << yr << endl;

			auto mustStop = [&](int dx, int dy) {
				return dx <= xl || xr <= dx || dy <= yl || yr <= dy;
			};

			vector<pair<int, int>> xyTrue;
			rep(x, width) rep(y, height) {
				if (oppPos[x][y] || !isWater[x][y]) continue;

				bool ok = false;
				rep1(i, 4) {
					if (0 <= x - i && oppPos[x - i][y] && isWater[x - i][y] && !mustStop(-i, 0)) { ok = true; break; }
					if (0 > x - i || !isWater[x - i][y] || mustStop(-i, 0) || inferOppGone.gone[x - i][y]) break;
				}
				if (!ok) rep1(i, 4) {
					if (x + i < width && oppPos[x + i][y] && isWater[x + i][y] && !mustStop(i, 0)) { ok = true; break; }
					if (width <= x + i || !isWater[x + i][y] || mustStop(i, 0) || inferOppGone.gone[x + i][y]) break;
				}
				if (!ok) rep1(i, 4) {
					if (0 <= y - i && oppPos[x][y - i] && isWater[x][y - i] && !mustStop(0, -i)) { ok = true; break; }
					if (0 > y - i || !isWater[x][y - i] || mustStop(0, -i) || inferOppGone.gone[x][y - i]) break;
				}
				if (!ok) rep1(i, 4) {
					if (y + i < height && oppPos[x][y + i] && isWater[x][y + i] && !mustStop(0, i)) { ok = true; break; }
					if (height <= y + i || !isWater[x][y + i] || mustStop(0, i) || inferOppGone.gone[x][y + i]) break;
				}

				if (ok) xyTrue.push_back({ x,y });
			}

			for (auto xy : xyTrue) oppPos[xy.first][xy.second] = true;
			//debug2D(inferOppGone.gone); cerr << endl;
			//debug2D(oppPos);
			inferOppGone.silence(oppPos, turn);
		}
		else if (ss[0] == "MINE") {
			inferOppGone.mine(turn);
		}
		else if (ss[0] == "TRIGGER") {
			int x = stoi(ss[1]), y = stoi(ss[2]);
			inferOppGone.inferGoneByMine(oppPos, isWater, x, y);
			preOppAttackPos.push_back({x,y});
		}
		else if (ss[0] == "MSG") {
		}
		else if (ss[0] == "NA") cerr << "最初" << endl;
		else { cerr << "inferPos : 命令漏れ " << ord << endl; }
	}

	inferOppGone.inferGoneByPos(oppPos);
}

int main() {
	int width;
	int height;
	int myId;
	cin >> width >> height >> myId; cin.ignore();

	chrono::system_clock::time_point start;
	start = chrono::system_clock::now();

	defField1(isWater);
	defField1(oppPos);
	defField1(myPos);
	for (int y = 0; y < height; y++) {
		string line;
		getline(cin, line);

		rep(x, width) {
			isWater[x][y] = (line[x] == '.');
			oppPos[x][y] = myPos[x][y] = (line[x] == '.');
		}
	}

	// Write an action using cout. DON'T FORGET THE "<< endl"
	// To debug: cerr << "Debug messages..." << endl;

	//TODO : 場所の広さを調べる

	//初期位置
	defField2(_gone, false);
	int centerMyX = 0, centerMyY = 0, ctmv = 0;
	rep(y, height) {
		rep(x, width) {
		  //cerr << countMovable(isWater, _gone, x, y) << endl;
			if (chmax(ctmv, countMovable(isWater, _gone, x, y))) {
				centerMyX = x; centerMyY = y;
			}
		}
	}
	cout << centerMyX << " " << centerMyY << endl;

	defField2(gone, false);
	set<pair<int, int>> myMinePos;
	int sonarResultSector = -1;
	int turn = 0;
	int preMyLife = 6, preOppLife = 6;
	vector<pair<int,int>> preMyAttackPos, preOppAttackPos;
	Gone oppGone(height, width), myGone(height, width);
	State myState = Early;

	cerr << "start : ";
	writeTime(start);
	// game loop
	while (1) {
		int myX;
		int myY;
		int myLife;
		int oppLife;
		int torpedoCooldown;
		int sonarCooldown;
		int silenceCooldown;
		int mineCooldown;
		cin >> myX >> myY >> myLife >> oppLife >> torpedoCooldown >> sonarCooldown >> silenceCooldown >> mineCooldown; cin.ignore();

		start = chrono::system_clock::now();
		string sonarResult; //Y, N, NA
		cin >> sonarResult; cin.ignore();
		string opponentOrders; // MOVE N |TORPEDO 3 5 , SARFACE 3(sector)
		getline(cin, opponentOrders);

		///準備
		cerr << "turn = " << ++turn << endl;
		int startMyX = myX, startMyY = myY;
		vector<string> oppOrds;
		getOrders(oppOrds, opponentOrders);


		//相手の位置の推測
		inferPos(oppPos, oppGone, isWater, oppOrds, sonarResult, sonarResultSector, turn, preMyAttackPos, preOppAttackPos, preOppLife - oppLife);

		cerr << "oppPos" << endl;
		debug2D(oppPos);

		//count
		int myPosCnt[9] = { 0 }, oppPosCnt[9] = { 0 }, myPosCntAll = 0, oppPosCntAll = 0;
		rep(x, width) rep(y, height) {
			if (oppPos[x][y]) {
				oppPosCnt[posToSector(height, width, x, y) - 1]++;
			}
			if (myPos[x][y]) myPosCnt[posToSector(height, width, x, y) - 1]++; //ついでにかうんと
		}
		rep(i, 9) { myPosCntAll += myPosCnt[i]; oppPosCntAll += oppPosCnt[i]; }

		auto near = [height, width, myX, myY, &isWater, &oppPos](int depth) {
			defField2(done, false);
			queue<pair<pair<int, int>, int>> que;
			que.push({ {myX,myY}, 0 });
			while (!que.empty()) {
				pair<pair<int, int>, int> v = que.front();
				que.pop();
				int x = v.first.first, y = v.first.second, d = v.second;
				if (!isInWater(isWater, x, y) || done[x][y]) continue;
				if (d > depth) continue;
				done[x][y] = true;

				if (oppPos[x][y]) return true;
				que.push({ {x + 1,y},d + 1 });
				que.push({ {x - 1,y},d + 1 });
				que.push({ {x,y + 1},d + 1 });
				que.push({ {x,y - 1},d + 1 });
			}
			return false;
		};

		//状態の決定
		cerr << "myState = ";
		if (turn < EARLY_TURN) {
			myState = Early;
			cerr << "Early" << endl;
		}
		else if ((myLife <= 2 || preMyLife - myLife > 0) && near(6)) {
			myState = Danger;
			cerr << "Danger" << endl;
		}
		else {
			myState = Safe;
			cerr << "Safe" << endl;
		}

		vector<string> orders;

		cerr << "行動前 : ";
		writeTime(start);
		///行動
		gone[myX][myY] = true;

		///SONAR
		long long int oppPosCntProd = 1; rep(i, 9) oppPosCntProd *= oppPosCnt[i] + 1;
		if (sonarCooldown == 0 && oppPosCntProd >= SONAR_BORDER && turn > SONAR_START_TIME) {
			int maxSectorCnt = -1, maxSector = 0;
			rep(i, 9) if (chmax(maxSectorCnt, oppPosCnt[i])) maxSector = i;
			orders.push_back("SONAR " + to_string(maxSector + 1));
			sonarResultSector = maxSector + 1;
		}

		cerr << "SONAR : ";
		writeTime(start);

		///MOVE
		Dir moveDir[4] = { N, E, S, W }; //0123
		movePlan(moveDir, myState, oppPos, height, width, centerMyX, centerMyY, myX, myY);

		int movableCnt[4] = { 0 }, movableCntAll = 0;
		rep(i, 4) movableCntAll += movableCnt[i] = countMovable(isWater, gone, myX + dirX[i], myY + dirY[i]);

		if (movableCntAll <= 0) {
		  bool moved = false;
		  rep(dr,4) if (canMove(isWater, gone, myX + dirX[moveDir[dr]], myY + dirY[moveDir[dr]])) {
				move(orders, moveDir[dr], myState, torpedoCooldown, sonarCooldown, silenceCooldown, mineCooldown);
				myX += dirX[moveDir[dr]]; myY += dirY[moveDir[dr]];
				moved = true;
				break;
			}
			if (!moved){
				centerMyX = myX; centerMyY = myY;
				orders.push_back("SURFACE");
				rep(x, width) rep(y, height) gone[x][y] = false;
			}
		}
		else {
			int notZeroDir = 0; rep(i, 4) notZeroDir += movableCnt[i] != 0;
			rep(i, 4) if (movableCnt[moveDir[i]] >= LARGE_AREA || movableCnt[moveDir[i]] >= movableCntAll / notZeroDir) {
				move(orders, moveDir[i], myState, torpedoCooldown, sonarCooldown, silenceCooldown, mineCooldown);
				myX += dirX[moveDir[i]]; myY += dirY[moveDir[i]];
				break;
			}
		}

		gone[myX][myY] = true;

		cerr << "MOVE : ";
		writeTime(start);

		///TORPEDO
				//魚雷を打つ場所の決定
		vector<pair<int, int>> oppPosNear;
		vector<pair<int, int>> torCand;
		defField2(done, false);
		queue<pair<pair<int, int>, int>> que;
		que.push({ {myX,myY},0 });
		while (!que.empty()) {
			pair<pair<int, int>, int> v = que.front();
			que.pop();
			int x = v.first.first, y = v.first.second, d = v.second;

			if (isInField(height, width, x, y) && oppPos[x][y] && d <= 4) torCand.push_back({ x,y });

			if (!isInField(height, width, x, y) || done[x][y] || d > 4) continue;

			if (oppPos[x][y]) {
				stack<pair<int, int>> st;
				st.push({ x,y });
				while (!st.empty()) {
					pair<int, int> v = st.top();
					int xx = v.first, yy = v.second;
					st.pop();

					if (!isInField(height, width, xx, yy) || done[xx][yy] || !oppPos[xx][yy]) continue;
					done[xx][yy] = true;
					oppPosNear.push_back({ xx,yy });
					st.push({ xx + 1,yy });
					st.push({ xx - 1,yy });
					st.push({ xx,yy + 1 });
					st.push({ xx,yy - 1 });
				}
			}

			done[x][y] = true;
			que.push({ {x + 1,y},d + 1 });
			que.push({ {x - 1,y},d + 1 });
			que.push({ {x,y + 1},d + 1 });
			que.push({ {x,y - 1},d + 1 });
		}

		int torpedoX = -1, torpedoY = -1;
		bool lockOnOpp = false;
		for (auto torxy : torCand) {
		  if (oppPosNear.empty() || oppPosNear.size() > 9) break;
			bool ok = true;
			for (auto xy : oppPosNear) if (!(abs(xy.first - torxy.first) <= 1 && abs(xy.second - torxy.second) <= 1)) { ok = false; break; }
			if (!ok) continue;
			lockOnOpp = true;
			torpedoX = torxy.first;
			torpedoY = torxy.second;
			cerr << "LOCK ON!!!" << endl;
		}
		cerr << "torCand : "; debugP(torCand);
		cerr << "oppPosNear : "; debugP(oppPosNear)

		//魚雷を落とせる範囲
		bool didTopedo = false;
		defField1(torpedoRange);
		setTorpedoRange(torpedoRange, isWater, myX, myY);
		if (lockOnOpp) {
			cerr << "攻撃準備 : " << torpedoX << " " << torpedoY <<  endl;
			if (torpedoCooldown == 0 && torpedoRange[torpedoX][torpedoY] && canFire(oppLife, myLife, myX, myY, torpedoX, torpedoY)) {
				torpedo(torpedoRange, orders, torpedoX, torpedoY);
				didTopedo = true;
			}
		}

		cerr << "TORPEDO : ";
		writeTime(start);

		///SILENCE
		if (silenceCooldown == 0 && (myState == Danger || didTopedo || myPosCntAll <= SILENCE_MYPOSCNT)) {
			int cnt[4] = { 0 }, dist[4] = { 0 };
			cerr << didTopedo << " " << myPosCntAll << " " << SILENCE_MYPOSCNT << endl;
			auto cntMovable = [myX, myY, &isWater](fieldB goneWill, int dir, int dis) {
				if (dirX[dir] == 0) rep(ds, dis) if (isInWater(isWater, myX,myY + ds * dirY[dir])) goneWill[myX][myY + ds * dirY[dir]] = true;
				if (dirY[dir] == 0) rep(ds, dis) if (isInWater(isWater, myX + ds * dirX[dir], myY)) goneWill[myX + ds * dirX[dir]][myY] = true;
				return countMovable(isWater, goneWill, myX + dis * dirX[dir], myY + dis * dirY[dir]);
			};

			bool n = true, e = true, s = true, w = true;
			if (near(6) && (preMyLife - myLife > 0 || didTopedo)) rep1(dis, 4) {
				if (n) { if (cnt[N]-1 <= cntMovable(gone, N, dis) && canMove(isWater, gone, myX, myY + dis)) {dist[N] = dis; cnt[N] = cntMovable(gone, N, dis);} else n = false; }
				if (e) { if (cnt[E]-1 <= cntMovable(gone, E, dis) && canMove(isWater, gone, myX + dis, myY)) {dist[E] = dis; cnt[E] = cntMovable(gone, E, dis);} else e = false; }
				if (s) { if (cnt[S]-1 <= cntMovable(gone, S, dis) && canMove(isWater, gone, myX, myY + dis)) {dist[S] = dis; cnt[S] = cntMovable(gone, S, dis);} else s = false; }
				if (w) { if (cnt[W]-1 <= cntMovable(gone, W, dis) && canMove(isWater, gone, myX - dis, myY)) {dist[W] = dis; cnt[W] = cntMovable(gone, W, dis);} else w = false; }
			}

			int mx = -1, mxDir = -1, mxDist = 0;
			rep(i, 4) if (chmax(mx, cnt[i])) { mxDir = i; mxDist = dist[i]; }

			if (mx > 0) {
				int x = myX, y = myY;
				while (abs(x - myX) <= abs(mxDist * dirX[mxDir]) && abs(y - myY) <= abs(mxDist * dirY[mxDir])) {
					gone[x][y] = true;
					y += dirY[mxDir];
					x += dirX[mxDir];
				}
				myX += mxDist * dirX[mxDir]; myY += mxDist * dirY[mxDir];
			}
			silence(orders, itod(mxDir), mxDist);
		}

		gone[myX][myY] = true;

		cerr << "SILENCE : ";
		writeTime(start);

		//MINE->TRIGGERのコンボ
		bool mineTriggerSkip = false;
		if (mineCooldown == 0 && oppLife <= 2 && myLife >= 2 && oppPosCntAll == 1) {
			int ox = -1, oy = -1;
			rep(x, width) rep(y, height) if (oppPos[x][y]) { ox = x; oy = y; break; }
			rep(dir, 4) {
				if (ox == myX + dirX[dir] && oy == myY + dirY[dir]) {
					switch (dir) {
					case 0: orders.push_back("MINE N"); break;
					case 1: orders.push_back("MINE E"); break;
					case 2: orders.push_back("MINE S"); break;
					case 3: orders.push_back("MINE W"); break;
					default:
						break;
					}
					orders.push_back("TRIGGER " + to_string(ox) + " " + to_string(oy));
					mineTriggerSkip = true;
				}
			}
		}
		if (!mineTriggerSkip) {
			///MINE
			if (mineCooldown == 0) {
				if (isInField(height, width, myX, myY + 1) && isWater[myX][myY + 1]) { myMinePos.insert({ myX, myY + 1 }); orders.push_back("MINE S"); }
				else if (isInField(height, width, myX, myY - 1) && isWater[myX][myY - 1]) { myMinePos.insert({ myX, myY - 1 }); orders.push_back("MINE N"); }
				else if (isInField(height, width, myX + 1, myY) && isWater[myX + 1][myY]) { myMinePos.insert({ myX + 1, myY }); orders.push_back("MINE E"); }
				else if (isInField(height, width, myX - 1, myY) && isWater[myX - 1][myY]) { myMinePos.insert({ myX - 1, myY }); orders.push_back("MINE W"); }
			}

			cerr << "MINE : ";
			writeTime(start);

			///TRIGGER
			for (set<pair<int, int>>::iterator it = myMinePos.begin(), end = myMinePos.end(); it != end; ++it) {
				pair<int, int> xy = *it;
				int mineX = xy.first, mineY = xy.second;

				int cntNear = 0;
				defField2(done,false);
				stack<pair<int,int>> st;
				st.push({mineX, mineY});
				while (!st.empty()) {
					pair<int,int> v = st.top();
					st.pop();
					int x = v.first, y = v.second;

					if (cntNear > 9) break;
					if (!isInField(height, width, x, y) || done[x][y] || !oppPos[x][y]) continue;
					done[x][y] = true;
					cntNear++;
					st.push({x+1,y});
					st.push({ x - 1,y });
					st.push({ x,y +1});
					st.push({ x,y -1});
				}
				
				int hitCnt = 0, bool triggered = false;
				for (int x = -1; x <= 1; ++x) for (int y = -1; y <= 1; ++y) if (isInField(height, width, mineX + x, mineY + y) && oppPos[mineX + x][mineY + y]) hitCnt++;
				if (canFire(oppLife, myLife, myX, myY, mineX, mineY) && (cntNear > 0 && cntNear <= hitCnt && myMinePos.size() > 3 || oppPosCntAll <= hitCnt)) {
					orders.push_back("TRIGGER " + to_string(mineX) + " " + to_string(mineY));
					myMinePos.erase(it);
					triggered = true;
					break;
				}
			}

			cerr << "TRIGGER : ";
			writeTime(start);
		}

		orders.push_back("MSG " + to_string(myPosCntAll) + ", " + to_string(oppPosCntAll) + " : " + to_string(myState));

		writeOrders(orders);

		//相手から見た自分の位置の推測
		string oppSonarResult = "NA";
		int oppSonarResultSector = -1;
		for (string s : oppOrds) if (s[0] == 'S' && s[1] == 'O') oppSonarResultSector = s[6] - '0';
		for (string& s : orders) if (s == "SURFACE") s += " " + to_string(posToSector(height, width, startMyX, startMyY));
		if (oppSonarResultSector >= 1) {
			if (posToSector(height, width, startMyX, startMyY) == oppSonarResultSector) oppSonarResult = "Y";
			else oppSonarResult = "N";
		}
		inferPos(myPos, myGone, isWater, orders, oppSonarResult, oppSonarResultSector, turn, preOppAttackPos, preMyAttackPos, preMyLife - myLife);
		cerr << "myPos" << endl;
		debug2D(myPos);

		preMyLife = myLife;
		preOppLife = oppLife;

		cerr << "end : ";
		writeTime(start);
	}
}
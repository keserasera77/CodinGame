#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iomanip>

using namespace std;

#define rep(i,n) for(int i = 0; i < (n); ++i)
#define rrep(i,n) for(int i = n-1; i >= 0; --i)
#define rep1(i,n) for(int i = 1; i <= (n); ++i)
#define rrep1(i,n) for(int i = (n); i >= 1; --i)
#define all(vec) (vec).begin(),(vec).end()

#define debug(vec) for(auto v : vec) cerr << v << " "; cerr << endl;
#define debug2D(vec2D) for(auto vec : vec2D) { for (auto v : vec) cerr << v << " "; cerr << endl; } 
#define debugP(vec) for(auto v : vec) cerr << "(" << v.first << "," << v.second << ") "; cerr << endl;
#define debug2DP(vec2D) for(auto vec : vec2D) { for (auto v : vec) cerr << "(" << v.first << "," << v.second << ") "; cerr << endl; } 

typedef long long ll;

constexpr ll INF = 1000000000; //10^10

template<class T>inline bool chmax(T& a, T b) { if (a <= b) { a = b; return true; } else return false; }
template<class T>inline bool chmin(T& a, T b) { if (a > b) { a = b; return true; } else return false; }

#define debugField(vec) for(int i = 0, n = vec.size(); i < n; ++i) {cerr << vec[i] << " "; if (i % W == W-1) cerr << endl;}

struct Game;
struct Pac;
struct Pacs;

const bool DEBUG_MODE = false;
const bool DEBUG_EVAL = false;
const double DEBUG_ROUND = 10;

unsigned int W; // size of the grid
unsigned int H; // top left corner is (x=0, y=0)

//敵の位置推定がまだ不正確なので
constexpr double EVAL_NEAR = 1.1;
constexpr int _EVAL_DIR_DEPTH = 10;

int dirX[4] = { 0,1,0,-1 }, dirY[4] = { -1, 0, 1, 0 };

void myAssert(bool cond, string msg) {
	if (!cond && !DEBUG_MODE) cerr << "** Assert!! ** " << msg << endl;
}

void NG(string msg) {
	myAssert(false, "NG : " + msg);
}

enum State {
	NORMAL, KILL, ESCAPE
};

enum Dir {
	U, R, D, L
};

Dir itod(int i) {
	switch (i) {
	case 0: return U; break;
	case 1: return R; break;
	case 2: return D; break;
	case 3: return L; break;
	default:
		NG("idod : " + to_string(i));
		break;
	}
}

enum Type {
	ROCK, PAPER, SCISSORS, DEAD
};

string to_string(Type type) {
	switch (type) {
	case ROCK: return "ROCK";
		break;
	case PAPER: return "PAPER";
		break;
	case SCISSORS: return "SCISSORS";
		break;
	case DEAD: return "DEAD";
		break;
	default:
		NG("to_string_Type");
		break;
	}
}

Type stot(string s) {
	if (s == "ROCK") return ROCK;
	else if (s == "PAPER") return PAPER;
	else if (s == "SCISSORS") return SCISSORS;
	else if (s == "DEAD") return DEAD;
	else NG("stot : " + s);
}

Type itot(int i) {
	if (i == 0) return ROCK;
	else if (i == 1) return PAPER;
	else if (i == 2) return SCISSORS;
	else if (i == 3) return DEAD;
	else NG("itot : " + to_string(i));
}

Type strongerType(Type t) {
	switch (t) {
	case ROCK: return PAPER;
		break;
	case PAPER: return SCISSORS;
		break;
	case SCISSORS: return ROCK;
		break;
	case DEAD:
		break;
	default:
		break;
	}
}

bool inField(int x, int y) {
	return 0 <= x && x < W && 0 <= y && y < H;
}

bool inField(int pos) {
	return 0 <= pos && pos < W * H;
}

pair<int, int> toXY(int pos) {
	myAssert(inField(pos), "toXY : pos = " + to_string(pos));
	return { pos % W, pos / W };
}
int fromXY(int x, int y) {
	if (x == -1) x = W - 1;
	else if (x == W) x = 0;
	myAssert(inField(x, y), "fromXY : x,y = " + to_string(x) + "," + to_string(y));
	return y * W + x;
}

int nextToDir(int pos, Dir dir) {
	return fromXY(toXY(pos).first + dirX[dir], toXY(pos).second + dirY[dir]);
}

struct Pac {
	int mId;
	Type mType;
	State mState;
	Type mWillSwitch;
	int mTargetId;
	int mSTL;
	int mACD;
	bool mSaveMode;
	int mPrePos;
	int mPos;
	int mNext;
	int mGoal;
	double mGoalE;
	vector<double> mEvalStage;
	queue<int> mSuperPellet;
	int mLastSeenTurn;

	Pac() : mId(-1), mTargetId(-1), mSaveMode(false), mState(NORMAL), mWillSwitch(DEAD), mPrePos(-1), mPos(-1), mNext(-1), mGoal(-1), mGoalE(-1), mType(DEAD), mSTL(-1), mACD(-1), mEvalStage(H* W), mLastSeenTurn(0) {}

	void debugEvalStage() {
		rep(i, H * W) {
			if (mEvalStage[i] == -1) cerr << right << setw(3) << "" << " ";
			else cerr << right << setw(3) << round(DEBUG_ROUND * mEvalStage[i]) / DEBUG_ROUND << " ";
			if (i % W == W - 1) cerr << endl;
		}
	}

	void debugPac() {
		if (DEBUG_MODE) return;
		int nx = toXY(mPos).first, ny = toXY(mPos).second;
		cerr << "id = " << mId << " " + to_string(mType) << " lastSeenTurn = " << mLastSeenTurn << ", pos = " << mPos << ", x = " << nx << ", y = " << ny << endl
			<< ", goal = " << toXY(mGoal).first << "," << toXY(mGoal).second << ", goalE = " << mGoalE
			<< ", next = " << toXY(mNext).first << "," << toXY(mNext).second << endl;
	}

	void set(int i, int x, int y, Type t, int s, int a, int turn) {
		mPrePos = mPos;
		mId = i;
		mPos = fromXY(x, y);
		mGoalE = mNext = mGoal = -1;
		mType = t;
		mSTL = s;
		mACD = a;
		mLastSeenTurn = turn;
	}

	bool isDead() { return mType == DEAD; }

	bool win(Pac& opp) {
		return strongerType(opp.mType) == mType;
	}


	bool collideInDangerSituation(Game& game);
	int arriveTurns(Game& game, int pos);
	void eval(Game& game);
	pair<double, vector<int>> determinePath(Game& game, vector<int> path, double evaluation, int depth, double& nowMax);
	void moveAI(Game& game);
	void move(Game& game);
	void ability(Game& game);
};

struct Pacs {
	int pacsN;
	vector<Pac> pacs;

	Pacs() : pacs(5), pacsN(0) {}

	Pac& operator[](int i) {
		return pacs[i];
	}

	void action(Game& game);
};

struct Game {
	chrono::system_clock::time_point startTime;

	int EVAL_DIR_DEPTH;

	int turn;
	int N;
	int myScore;
	int oppScore;
	int visiblePacCount;
	int visiblePelletCount;

	Pacs myPacs;
	Pacs oppPacs;

	//壁は-1,0以上はペレットの得点。
	vector<double> stage;
	vector<double> evalStage;
	vector<int> fromEnemy;
	vector<vector<int>> distance;
	vector<int> exit;
	vector<int> degree;
	vector<int> inferOppPos;

	vector<int> pellet10;

	Game() : EVAL_DIR_DEPTH(_EVAL_DIR_DEPTH), visiblePacCount(-1), visiblePelletCount(-1), turn(0), N(W* H), myScore(0), oppScore(0),
		inferOppPos(N, -1), exit(N, -1), degree(N, 0), stage(N), evalStage(N), fromEnemy(N, N), pellet10(), myPacs(), oppPacs(), distance(N, vector<int>(N)) {};

	void timeStart() { startTime = chrono::system_clock::now(); }

	void writeTime() {
		chrono::system_clock::time_point end = chrono::system_clock::now();
		cerr << static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - startTime).count() / 1000.0) << "[ms]" << endl;
	}

	double getTime() {
		chrono::system_clock::time_point end = chrono::system_clock::now();
		return static_cast<double>(chrono::duration_cast<chrono::microseconds>(end - startTime).count() / 1000.0);
	}

	void build() {
		//全点対間の距離
		rep(r, N - W + 1) {
			vector<bool> done(N, false);
			queue<pair<int, int>> que;
			que.push({ 0,r });
			while (!que.empty()) {
				int d = que.front().first, pos = que.front().second;
				que.pop();

				if (!canMove(pos) || done[pos]) continue;
				done[pos] = true;

				//if (canMove(r)) cerr << r << " -> " << pos << " : " << d << endl;
				distance[r][pos] = d;

				rep(dir, 4) que.push({ d + 1, nextToDir(pos, itod(dir)) });
			}
		}

		//次数
		rep(pos, N - W + 1) if (canMove(pos)) rep(dir, 4) if (canMove(nextToDir(pos, itod(dir)))) degree[pos]++;

		//行き止まり
		rep(r, N - W + 1) {
			vector<int> route;
			queue<int> que;
			if (exit[r] == -1 && degree[r] == 1) que.push(r);
			while (!que.empty()) {
				int pos = que.front();
				que.pop();

				if (!canMove(pos)) continue;

				route.push_back(pos);

				if (degree[pos] >= 3) break;

				rep(dir, 4) if (nextToDir(pos, itod(dir)) != route[max((int)route.size() - 2, 0)]) que.push(nextToDir(pos, itod(dir)));
			}

			for (int p : route) exit[p] = route.back();
		}

		if (!DEBUG_MODE) { cerr << "build end : "; writeTime(); }
	}

	void debugEvalStage() {
		rep(i, N) {
			if (evalStage[i] == -1)  cerr << right << setw(3) << "" << " ";
			else cerr << right << setw(3) << round(DEBUG_ROUND * evalStage[i]) / DEBUG_ROUND << " ";
			if (i % W == W - 1) cerr << endl;
		}
	}

	void resetEvalStage() {
		rep(i, N) evalStage[i] = stage[i];
	}

	void pellet10Reset() {
		for (int pos : pellet10) evalStage[pos] = stage[pos] = 0;
		pellet10.clear();
	}

	void resetVisible();

	void setScore(int myS, int oppS) {
		myScore = myS; oppScore = oppS;
	}

	void putPellet(int x, int y, int value) {
		int pos = fromXY(x, y);
		if (inField(pos)) { evalStage[pos] = stage[pos] = value; /*evalStage[pos] += 0.1;*/ }
	}

	bool canMove(int x, int y) {
		if (x == -1) x = W - 1;
		else if (x == W) x = 0;
		return inField(x, y) && stage[fromXY(x, y)] >= 0;
	}

	bool canMove(int pos) {
		int x = toXY(pos).first, y = toXY(pos).second;
		if (x == -1) x = W - 1;
		else if (x == W) x = 0;
		return inField(x, y) && stage[fromXY(x, y)] >= 0;
	}

	void eat(int x, int y) {
		int pos = fromXY(x, y);
		if (inField(x, y)) evalStage[pos] = stage[pos] = 0;
	}

	void allocSuperPellet() {
		vector<priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>>> epp(5);
		for (Pac& pac : myPacs.pacs) for (int p : pellet10) if (!pac.isDead()) epp[pac.mId].push({ distance[p][pac.mPos], p });

		map<int, int> used;
		int cnt = 0;
		while (!epp.empty()) {
			int dist = N, pPos = -1;
			Pac* pac = 0;
			rep(i, 5) if (!myPacs[i].isDead() && !epp[i].empty() && chmin(dist, epp[i].top().first)) {
				pPos = epp[i].top().second;
				pac = &myPacs.pacs[i];
			}

			epp[pac->mId].pop();

			if (cnt == 4) break;
			if (used[pPos]) continue;
			pac->mSuperPellet.push(pPos);
			used[pPos] = pac->mId + 1;
			cnt++;

			while (!epp[pac->mId].empty()) epp[pac->mId].pop();
			for (int p : pellet10) if (!used[p]) epp[pac->mId].push({ dist + distance[pPos][p], p });
		}
	}

	void inferPos() {
		for (Pac& opac : oppPacs.pacs) {
			if (opac.isDead() || turn - opac.mLastSeenTurn > 0) continue;

			if (degree[opac.mPos]);
		}
	}

	int toGoalNextPos(int goal, int start) {
		int n = stage.size();

		vector<int> prePos(n);
		vector<bool> done(n, false);
		queue<pair<int, int>> que;
		que.push({ start, start });
		while (!que.empty()) {
			int par = que.front().first, v = que.front().second;
			que.pop();

			if (!canMove(v) || done[v]) continue;
			done[v] = true;

			prePos[v] = par;
			if (v == goal) break;

			rep(d, 4) {
				int w = nextToDir(v, itod(d));
				que.push({ v,w });
			}
		}

		int pos = goal;
		while (prePos[pos] != start) {
			pos = prePos[pos];
		}

		return pos;
	}

	void eval();
};

bool Pac::collideInDangerSituation(Game& game) {
	bool collide = false;
	for (Pac& opac : game.oppPacs.pacs) {
		if (opac.isDead() || game.turn - opac.mLastSeenTurn > 0) continue;
		if (opac.win(*this) && game.distance[mPos][opac.mPos] <= 1 + (opac.mSTL > 0) + (mSTL > 0)) { collide = true; break; }
	}

	return collide;
}

int Pac::arriveTurns(Game& game, int pos) {
	return game.distance[mPos][pos] - mSTL;
}

void Pacs::action(Game& game) {
	if (pacsN == 1) game.EVAL_DIR_DEPTH = _EVAL_DIR_DEPTH + 5;
	if (pacsN == 2) game.EVAL_DIR_DEPTH = _EVAL_DIR_DEPTH + 3;
	if (pacsN >= 4) game.EVAL_DIR_DEPTH = _EVAL_DIR_DEPTH - 1;

	//game.inferPos();
	game.eval();
	if (!DEBUG_MODE) { cerr << "eval end : "; game.writeTime(); }

	if (game.turn == 1) game.allocSuperPellet();

	for (Pac& pac : pacs) {
		if (pac.isDead()) continue;

		pac.eval(game);

		pac.moveAI(game);
		if (DEBUG_EVAL)  pac.debugEvalStage();
		if (!DEBUG_MODE) { cerr << pac.mId << "'s Evaluation end : "; game.writeTime(); }
	}

	for (Pac& pac : pacs) {
		if (pac.isDead()) continue;

		if (pac.mACD == 0 && !pac.collideInDangerSituation(game)) pac.ability(game);
		else pac.move(game);
	}

	if (!DEBUG_MODE) { cerr << "allAction end : "; game.writeTime(); }
}

void Game::resetVisible() {
	for (Pac& pac : myPacs.pacs) {
		if (pac.isDead()) continue;
		rep(dir, 4) {
			int pos = pac.mPos;
			int cnt = 0;
			while (canMove(pos) && cnt <= W) {
				++cnt;
				stage[pos] = evalStage[pos] = 0;
				pos = nextToDir(pos, itod(dir));
			}
		}
	}
}

void Game::eval() {
	vector<bool> done(N, false);

	if (turn > 400 / visiblePacCount) rep(i, N) {
		vector<int> comp;
		queue<int> que;
		if (canMove(i) && stage[i] >= 0 && !done[i]) que.push(i);
		else continue;

		while (!que.empty()) {
			int v = que.front();
			que.pop();

			if (!canMove(v) || stage[i] < 0 || done[v]) continue;
			done[v] = true;

			comp.push_back(v);

			rep(dir, 4) que.push(nextToDir(v, itod(dir)));
		}

		for (int p : comp) evalStage[p] *= (double)comp.size() / (visiblePelletCount + 1) + 1.0;
	}


	rep(pos, N) for (Pac& pac : oppPacs.pacs) {
		if (pac.isDead() || turn - pac.mLastSeenTurn > 0) continue;
		chmin(fromEnemy[pos], distance[pos][pac.mPos]);
	}
}

void Pac::eval(Game& game) {
	if (isDead()) return;

	rep(i, game.N - W + 1) mEvalStage[i] = game.evalStage[i];

	//自分のパックの行き先に近い場所は減点
	for (Pac& pac : game.myPacs.pacs) {
		if (mId == pac.mId || pac.isDead() || pac.mGoal == -1) continue;

		vector<bool> done(game.N, false);
		queue<pair<int, int>> que;
		que.push({ 0,pac.mGoal });
		while (!que.empty()) {
			int d = que.front().first, pos = que.front().second;
			que.pop();

			if (!game.canMove(pos) || done[pos] || d > game.EVAL_DIR_DEPTH) continue;
			done[pos] = true;

			mEvalStage[pos] *= (double)d / game.EVAL_DIR_DEPTH;

			rep(i, 4) que.push({ d + 1,nextToDir(pos, itod(i)) });
		}
	}

	//各方向について評価
	vector<int> fromOthers(game.N);
	rep(pos, game.N - W + 1) for (Pac& mpac : game.myPacs.pacs) if (mpac.mId != mId && !mpac.isDead()) fromOthers[pos] = min(game.fromEnemy[pos], game.distance[mpac.mPos][pos]);
	rep(pos, game.N - W + 1) if (game.canMove(pos) && game.distance[pos][mPos] <= fromOthers[pos]) mEvalStage[pos] *= EVAL_NEAR;
}


pair<double, vector<int>> Pac::determinePath(Game& game, vector<int> path, double evaluation, int depth, double& nowMax) {
	int pos = path.back();
	if (depth > game.EVAL_DIR_DEPTH || game.getTime() > 45 + mId) return { evaluation, path };
	if (!game.canMove(pos)) return { -1, {} };

	map<int, unsigned short int> mp;
	rrep(i, depth + 1) {
		if ((++mp[path[i]]) > 2) return { evaluation, path };
	}

	if (mp[path.back()] == 1) evaluation += mEvalStage[pos];

	//if (find(path.rbegin() + 1, path.rend(), pos) == path.rend()) evaluation += mEvalStage[pos];

	chmax(nowMax, evaluation);
	if ((evaluation + 1.1 * (game.EVAL_DIR_DEPTH - depth - game.pellet10.size()) + game.pellet10.size() * 10.0) < nowMax)
		return { evaluation, path };

	pair<int, vector<int>> ret = { evaluation, path };

	rep(dir, 4) {
		int nPos = nextToDir(pos, itod(dir));
		path.push_back(nPos);
		auto ret0 = determinePath(game, path, evaluation, depth + 1, nowMax);
		if (ret.first / ((double)ret.second.size()) < ret0.first / ((double)ret0.second.size())) ret = ret0;
		path.pop_back();
	}

	//cerr << "eval = " << ret.first << ",path = ";
	//for (auto p : ret.second) cerr << p << " "; cerr << endl;

	return ret;
}

void Pac::moveAI(Game& game) {
	auto xy = toXY(mPos);
	const int x = xy.first, y = xy.second;

	//if (!mSuperPellet.empty()) cerr << mId << " : " << game.stage[mSuperPellet.front()] << endl;

	if (!mSuperPellet.empty() && (game.stage[mSuperPellet.front()] == 0)) mSuperPellet.pop();

	if (!mSuperPellet.empty()) {
		int pos = mSuperPellet.front();
		mGoal = pos;
		vector<int> prePos(game.N, -1);
		vector<bool> done(game.N, false);
		queue<pair<int, int>> que;
		que.push({ -1,mPos });
		while (!que.empty()) {
			int par = que.front().first, v = que.front().second;
			que.pop();

			if (!game.canMove(v) || done[v]) continue;
			done[v] = true;

			prePos[v] = par;
			if (v == mGoal) break;

			rep(i, 4) que.push({ v, nextToDir(v, itod(i)) });
		}

		int p = mGoal;
		while (prePos[p] != mPos) {
			p = prePos[p];
		}
		mNext = p;

		if (!DEBUG_MODE) debugPac();
		return;
	}

	if (mTargetId != -1 && game.oppPacs[mTargetId].mType == DEAD) mState = NORMAL;

	if (mState == KILL) {
		mGoal = game.oppPacs[mTargetId].mPos;
		mNext = game.toGoalNextPos(mGoal, mPos);
		return;
	}

	if (mSaveMode) {
		//幅優先で経路探索
		double cnt = 0;
		vector<double> vEval(game.N, 0);
		vector<int> prePos(game.N, -1);
		queue<pair<pair<double, pair<int, int>>, int>> que;
		que.push({ {0, { -1,mPos }}, 0 });
		while (!que.empty()) {
			int depth = que.front().second;
			double eval = que.front().first.first;
			int par = que.front().first.second.first, v = que.front().first.second.second;
			que.pop();

			if (!game.canMove(v) || prePos[v] > 0) continue;
			prePos[v] = par;

			eval += mEvalStage[v];
			vEval[v] = eval / (double)depth;

			chmax(cnt, eval);

			prePos[v] = par;

			rep(i, 4) que.push({ {eval, { v, nextToDir(v, itod(i)) }}, depth + 1 });
		}
		double maxE = 0;
		rep(pos, game.N) if (chmax(maxE, vEval[pos])) mGoal = pos;

		int p = mGoal;
		while (prePos[p] != mPos) p = prePos[p];
		mNext = p;

		if (cnt > 3 && cnt / vEval[mGoal] < 5) mSaveMode = false;
	}
	else {
		double nowMax = 0;
		auto evalPath = determinePath(game, { mPos }, 0, 0, nowMax);
		double eval = evalPath.first;
		vector<int> path = evalPath.second;



		if (eval < 2) mSaveMode = true;

		for (int pos : path) {
			game.evalStage[pos] = 0;
			//if (!DEBUG_MODE) cerr << "(" << toXY(pos).first << "," << toXY(pos).second << ") -> ";
		} //if (!DEBUG_MODE)cerr << endl;

		if (path.size() > 2) {
			mGoal = path[2];
			mNext = path[1];
		}
		else if (path.size() == 2) {
			mGoal = mNext = path[1];
		}
		else {
			//取り敢えず一番近い餌に行く
			vector<int> prePos(game.N, -1);
			queue<pair<int, int>> que;
			que.push({ -1,mPos });
			while (!que.empty()) {
				int par = que.front().first, v = que.front().second;
				que.pop();

				if (!game.canMove(v) || prePos[v] > 0) continue;
				prePos[v] = par;

				if (game.evalStage[v] > 0) { mGoal = v; break; }

				rep(i, 4) que.push({ v, nextToDir(v, itod(i)) });
			}
			int p = mGoal;
			while (prePos[p] != mPos) p = prePos[p];
			mNext = p;
		}

	}


	myAssert(mGoal != -1, "moveAI : goal == -1");

	//if (!DEBUG_MODE) debugPac();
}

void Pac::move(Game& game) {
	string msg;
	if (mGoal == mPos) {
		mGoal = mNext;
	}

	//キル
	vector<pair<int, pair<int, pair<int, Type>>>> cand;
	for (Pac& opac : game.oppPacs.pacs) {
		if (opac.isDead() || game.turn - opac.mLastSeenTurn > 0) continue;
		if (game.exit[opac.mPos] != -1) {
			cand.push_back({ opac.mId, {game.exit[opac.mPos], {opac.arriveTurns(game, game.exit[opac.mPos]), opac.mType}} });
		}
	}

	for (auto _pdt : cand) {
		int oid = _pdt.first;
		auto pdt = _pdt.second;
		if ((strongerType(pdt.second.second) == mType || mACD < pdt.second.first)
			&& (arriveTurns(game, game.exit[pdt.first]) < pdt.second.first)) {
			msg += "kill";
			mWillSwitch = strongerType(pdt.second.second);
			mState = KILL;
			mTargetId = oid;
			mGoal = game.oppPacs[oid].mPos;
			mNext = game.toGoalNextPos(mPos, mGoal);
		}
	}

	//衝突回避
	Pac* collidingPac = this;
	for (Pac& opac : game.oppPacs.pacs) {
		if (!opac.isDead() && game.turn - opac.mLastSeenTurn == 0
			&& (opac.win(*this) || opac.mACD == 0)
			&& (game.distance[opac.mPos][mNext] <= 1 + (opac.mSTL > 0) + (mSTL > 0))) {
			collidingPac = &opac; break;
		}
	}

	if (collidingPac == this) //一応速くなる
		for (Pac& mpac : game.myPacs.pacs) {
			if (!mpac.isDead() && mpac.mId != mId
				&& (mpac.mNext == mNext
					|| (mpac.mPos == mNext && mpac.mNext == mPos))) {
				collidingPac = &mpac; break;
			}
		}

	int distFromCol = 0;
	if (collidingPac != this) rep(dir, 4) {
		int nPos = nextToDir(mPos, itod(dir));
		if (game.canMove(nPos) && chmax(distFromCol, game.distance[nPos][collidingPac->mPos])) {
			mNext = mGoal = nPos;
			rep(dir2, 4) if (nextToDir(nPos, itod(dir2)) != mPos && game.canMove(nextToDir(nPos, itod(dir2)))) mGoal = nextToDir(nPos, itod(dir2));
			mGoalE = mEvalStage[mGoal];
			mState = ESCAPE;
			msg += "!";
		}
	}
	else if (mPos == mPrePos && mACD < 9 && mState != ESCAPE) rep(dir, 4) {
		int nPos = nextToDir(mPos, itod(dir));
		if (!game.canMove(nPos) || mNext == nPos) continue;

		mNext = mGoal = nPos;
		rep(dir2, 4) if (nextToDir(nPos, itod(dir2)) != mPos && game.canMove(nextToDir(nPos, itod(dir2)))) mGoal = nextToDir(nPos, itod(dir2));
		mGoalE = mEvalStage[mGoal];
		msg += "?";
		break;
	}

	cout << "MOVE " << mId << " " << toXY(mGoal).first << " " << toXY(mGoal).second << " (" << toXY(mGoal).first << "," << toXY(mGoal).second << ") " << msg << "|";
}

void Pac::ability(Game& game) {
	if (mACD != 0) { NG("acd == 0"); return; }

	if (mWillSwitch != DEAD) {
		cout << "SWITCH " << mId << " " << to_string(mWillSwitch) << "|";
		mWillSwitch = DEAD;
		return;
	}

	for (Pac& opac : game.oppPacs.pacs) {
		if (opac.isDead() || game.turn - mLastSeenTurn > 0 || win(opac)) continue;

		if (game.distance[mNext][opac.mPos] < 1 + (mSTL > 0) + (opac.mSTL > 0)) {
			cout << "SWITCH " << mId << " " << to_string(strongerType(opac.mType)) << "|";
			return;
		}
	}
	cout << "SPEED " << mId << "|";
}

int main() {
	cin >> W >> H; cin.ignore();
	if (DEBUG_MODE) cerr << W << " " << H << endl;

	Game game;
	for (int y = 0; y < H; y++) {
		string row;
		getline(cin, row); // one line of the grid: space " " is floor, pound "#" is wall
		if (DEBUG_MODE) cerr << row << endl;
		rep(x, W) if (row[x] == '#') game.putPellet(x, y, -1); else game.putPellet(x, y, 1);
	}

	game.build();

	// game loop
	while (1) {
		game.turn++;
		int myScore;
		int opponentScore;
		cin >> myScore >> opponentScore; cin.ignore();
		if (DEBUG_MODE) cerr << myScore << " " << opponentScore << endl;
		game.setScore(myScore, opponentScore);

		int visiblePacCount; // all your pacs and enemy pacs in sight
		cin >> visiblePacCount; cin.ignore();
		game.visiblePacCount = visiblePacCount;
		if (DEBUG_MODE) cerr << visiblePacCount << endl;

		game.myPacs.pacsN = 0;
		for (int i = 0; i < visiblePacCount; i++) {
			int pacId; // pac number (unique within a team)
			bool mine; // true if this pac is yours
			int x; // position in the grid
			int y; // position in the grid
			string typeId; // unused in wood leagues
			int speedTurnsLeft; // unused in wood leagues
			int abilityCooldown; // unused in wood leagues
			cin >> pacId >> mine >> x >> y >> typeId >> speedTurnsLeft >> abilityCooldown; cin.ignore();
			if (DEBUG_MODE) cerr << pacId << " " << mine << " " << x << " " << y << " " << typeId << " " << speedTurnsLeft << " " << abilityCooldown << endl;

			if (mine) {
				game.myPacs[pacId].set(pacId, x, y, stot(typeId), speedTurnsLeft, abilityCooldown, game.turn);
				++game.myPacs.pacsN;
				if (game.turn == 1) game.oppPacs[pacId].set(pacId, W - 1 - x, y, stot(typeId), speedTurnsLeft, abilityCooldown, game.turn);
			}
			else {
				game.oppPacs[pacId].set(pacId, x, y, stot(typeId), speedTurnsLeft, abilityCooldown, game.turn);
				++game.oppPacs.pacsN;
				if (stot(typeId) == DEAD) --game.oppPacs.pacsN; // TODO
			}
			game.eat(x, y);
		}

		int visiblePelletCount; // all pellets in sight
		cin >> visiblePelletCount; cin.ignore();
		game.visiblePelletCount = visiblePelletCount;
		if (DEBUG_MODE)  cerr << visiblePelletCount << endl;

		game.pellet10Reset();
		game.resetVisible();
		for (int i = 0; i < visiblePelletCount; i++) {
			int x;
			int y;
			int value; // amount of points this pellet is worth
			cin >> x >> y >> value; cin.ignore();
			if (DEBUG_MODE) cerr << x << " " << y << " " << value << endl;
			game.putPellet(x, y, value);

			if (value == 10) game.pellet10.push_back(fromXY(x, y));
		}

		game.timeStart();
		game.resetEvalStage();

		game.myPacs.action(game);

		cout << endl;
	}
}
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <queue>
#include "jsoncpp/json.h"
using namespace std;

const int BLACK = 1; //棋盘为黑
const int WHITE = -1; // 棋盘为白
const int EMPTY = 0; // 棋盘为空

const int CX[] = {-1, 0, 1, 0, -1, 1}; //搜索
const int CY[] = {0, -1, 0, 1, 1, -1};  //搜索
const int BOARD_LEN = 11; // 棋盘长度
int currentPlayer = BLACK; // 当前玩家
int board[BOARD_LEN][BOARD_LEN] = {0}; // 当前棋盘

bool inBoard(int x, int y){
    return x >= 0 && y >=0 && x < BOARD_LEN && y < BOARD_LEN;
}

//true: has air
bool bfs(int x1, int y1, int x2, int y2){
    queue<int> Q;
    bool dfsAirVisit[BOARD_LEN][BOARD_LEN] = {false};
    Q.push(x1 * BOARD_LEN + y1);
    dfsAirVisit[x1][y1] = true;
    while (!Q.empty()){
        int Vn = Q.front();
        Q.pop();
        for (int i = 0; i < 6; ++i) {
            int x = Vn/BOARD_LEN + CX[i];
            int y = Vn/BOARD_LEN + CY[i];
            if(inBoard(x, y)){
                if(board[x][y] == board[x1][y1]){
                    int Vw = x * BOARD_LEN + y;
                    if(Vw == x2 * BOARD_LEN + y2){
                        return true;
                    }
                    if(! dfsAirVisit[x][y]){
                        Q.push(Vw);
                        dfsAirVisit[x][y] = true;
                    }
                }
            }
        }
    }
    return false;
}

bool isRedOver(){
    bool flag[2] = {false};
    for (int i = 0; i < BOARD_LEN; ++i) {
        if(board[0][i] == BLACK){
            flag[0] = true;
            break;
        }
    }
    for (int i = 0; i < BOARD_LEN; ++i) {
        if(board[BOARD_LEN-1][i] == BLACK){
            flag[1] = true;
            break;
        }
    }
    if (!flag[0] || !flag[1]){
        return false;
    }
    int start[BOARD_LEN] = {-1};
    int end[BOARD_LEN] = {-1};
    for (int i = 0; i < BOARD_LEN; ++i) {
        if(board[0][i] == BLACK){
            start[i] = 1;
        }
        if(board[BOARD_LEN-1][i] == BLACK) {
            end[i] = 1;
        }
    }
    for (int i = 0; i < BOARD_LEN; ++i) {
        if(start[i] == 1){
            for (int j = 0; j < BOARD_LEN; ++j) {
                if (end[j] == 1){
                    if (bfs(0, i, 10, j)){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool isBlueOver(){
    bool flag[2] = {false};
    for (int i = 0; i < BOARD_LEN; ++i) {
        if(board[i][0] == BLACK){
            flag[0] = true;
            break;
        }
    }
    for (int i = 0; i < BOARD_LEN; ++i) {
        if(board[i][BOARD_LEN-1] == BLACK){
            flag[1] = true;
            break;
        }
    }
    if (!flag[0] || !flag[1]){
        return false;
    }
    int start[BOARD_LEN] = {-1};
    int end[BOARD_LEN] = {-1};
    for (int i = 0; i < BOARD_LEN; ++i) {
        if(board[i][0] == BLACK){
            start[i] = 1;
        }
        if(board[i][BOARD_LEN-1] == BLACK) {
            end[i] = 1;
        }
    }
    for (int i = 0; i < BOARD_LEN; ++i) {
        if(start[i] == 1){
            for (int j = 0; j < BOARD_LEN; ++j) {
                if (end[j] == 1){
                    if (bfs(i, 0, j, 10)){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool isGameOver(int player){
    if (player == BLACK){
        if (isRedOver()){
            return true;
        }
        return false;
    }
    if (player == WHITE){
        if (isBlueOver()){
            return true;
        }
        return false;
    }
}

bool ProcStep(int x, int y, int player, bool checkOnly = false)
{
    if (!inBoard(x, y) || board[x][y])
        return false;
    if (!checkOnly)
        board[x][y] = player;
    return true;
}

int main()
{
    string str;
    getline(cin, str);
    Json::Reader reader;
    Json::Value input, output;
    reader.parse(str, input);
    input = input["log"];

    currentPlayer = BLACK; // 先手为黑
    if (input.size() == 0)
    {
        output["display"] = "";
        output["command"] = "request";
        output["content"]["0"]["x"] = -1;
        output["content"]["0"]["y"] = -1;
    }
    else
    {
        for (int i = 1; i < input.size(); i += 2)
        {
            bool isLast = i == input.size() - 1;
            Json::Value content;
            Json::Value display;
            if (currentPlayer == BLACK) // 0号玩家 / 黑方
            {
                Json::Value answer = input[i]["0"]["response"].isNull() ? input[i]["0"]["content"] : input[i]["0"]["response"];
                if (((answer.isString() &&
                      reader.parse(answer.asString(), content)) ||
                     (answer.isObject() &&
                      (content = answer, true))) &&
                    content["x"].isInt() && content["y"].isInt()) // 保证输入格式正确
                {
                    int currX = content["x"].asInt();
                    int currY = content["y"].asInt();
                    if (!ProcStep(currX, currY, currentPlayer) && isLast) // 不合法棋步！
                    {
                        stringstream ss;
                        ss << "INVALID_MOVE  (" << currX << "," << currY << ")";
                        if (inBoard(currX, currY) && board[currX][currY])
                            ss << " is not empty";
                        else
                            ss << " is forbidden position";
                        string s; getline(ss, s);
                        output["display"]["err"] = s;
                        output["display"]["winner"] = 1;
                        output["command"] = "finish"; // 判输
                        output["content"]["0"] = 0;
                        output["content"]["1"] = 1;
                    }
                    else if (isLast) // 正常棋步
                    {
                        output["display"]["x"] = currX;
                        output["display"]["y"] = currY;
                        output["display"]["color"] = 0;
                        if (isGameOver(currentPlayer)) // 游戏结束
                        {
                            output["display"]["winner"] = 0;
                            output["command"] = "finish";
                            output["content"]["0"] = 1;
                            output["content"]["1"] = 0;
                        }
                        else
                        {
                            output["command"] = "request";
                            output["content"]["1"]["x"] = currX;
                            output["content"]["1"]["y"] = currY;
                        }
                    }
                }
                else if (isLast)
                {
                    output["display"]["err"] = "INVALID_INPUT_VERDICT_" + input[i]["0"]["verdict"].asString();
                    output["display"]["winner"] = 1;
                    output["command"] = "finish"; // 判输
                    output["content"]["0"] = 0;
                    output["content"]["1"] = 1;
                }
            }
            else
            {
                Json::Value answer = input[i]["1"]["response"].isNull() ? input[i]["1"]["content"] : input[i]["1"]["response"];
                if (((answer.isString() &&
                      reader.parse(answer.asString(), content)) ||
                     (answer.isObject() &&
                      (content = answer, true))) &&
                    content["x"].isInt() && content["y"].isInt()) // 保证输入格式正确
                {
                    int currX = content["x"].asInt();
                    int currY = content["y"].asInt();
                    if (!ProcStep(currX, currY, currentPlayer) && isLast) // 不合法棋步！
                    {
                        stringstream ss;
                        ss << "INVALID_MOVE  (" << currX << "," << currY << ")";
                        if (inBoard(currX, currY) && board[currX][currY])
                            ss << " is not empty";
                        else
                            ss << " is forbidden position";
                        string s; getline(ss, s);
                        output["display"]["err"] = s;
                        output["display"]["winner"] = 0;
                        output["command"] = "finish"; // 判输
                        output["content"]["0"] = 1;
                        output["content"]["1"] = 0;
                    }
                    else if (isLast) // 正常棋步
                    {
                        output["display"]["x"] = currX;
                        output["display"]["y"] = currY;
                        output["display"]["color"] = 1;
                        if (isGameOver(currentPlayer)) // 游戏结束
                        {
                            output["display"]["winner"] = 1;
                            output["command"] = "finish";
                            output["content"]["0"] = 0;
                            output["content"]["1"] = 1;
                        }
                        else
                        {
                            output["command"] = "request";
                            output["content"]["0"]["x"] = currX;
                            output["content"]["0"]["y"] = currY;
                        }
                    }
                }
                else if (isLast)
                {
                    output["display"]["err"] = "INVALID_INPUT_VERDICT_" + input[i]["1"]["verdict"].asString();
                    output["display"]["winner"] = 0;
                    output["command"] = "finish"; // 判输
                    output["content"]["0"] = 1;
                    output["content"]["1"] = 0;
                }
            }
            currBotColor *= -1;
        }
    }
    Json::FastWriter writer;
    cout << writer.write(output) << endl;
    return 0;
}
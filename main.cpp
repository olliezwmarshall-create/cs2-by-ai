#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cassert>
#include <cstdio>
constexpr double PI=3.141592653589793;
const char* world[]={"################","#..............#","#..##....##....#","#..............#","#......#.......#","#..#...#...##..#","#..#...........#","#......##......#","#..............#","#...##....#....#","#.........#....#","#..##..........#","#.......##.....#","#..............#","#..............#","################"};
bool solid(double x,double y){return x<0||y<0||x>=16||y>=16||world[int(y)][int(x)]=='#';}
struct Bot {double x,y; int hp; double cooldown;};
struct Game {
 double x=2,y=2,a=0,shot=0,reload=0; int hp=100,ammo=12,kills=0,round=0; bool won=false; std::vector<Bot> bots;
 Game(){reset();}
 void reset(){x=2;y=2;a=0;hp=100;ammo=12;kills=0;shot=reload=0;won=false;round++;bots={{12,2,100,2},{13,8,100,2},{8,13,100,2},{2,12,100,2},{10,6,100,2}};}
 double ray(double ox,double oy,double angle,double limit=24) const {for(double d=.02;d<limit;d+=.025)if(solid(ox+cos(angle)*d,oy+sin(angle)*d))return d;return limit;}
 void move(double dx,double dy){if(!solid(x+dx+.18,y)&&!solid(x+dx-.18,y)&&!solid(x+dx,y+.18)&&!solid(x+dx,y-.18))x+=dx;if(!solid(x+.18,y+dy)&&!solid(x-.18,y+dy)&&!solid(x,y+dy+.18)&&!solid(x,y+dy-.18))y+=dy;}
 void fire(){if(hp<=0||won||shot>0||reload>0||ammo<=0)return;ammo--;shot=.22;Bot* target=nullptr;double nearest=24;for(auto& b:bots){if(b.hp<=0)continue;double d=hypot(b.x-x,b.y-y),angle=atan2(b.y-y,b.x-x),diff=atan2(sin(angle-a),cos(angle-a));if(std::abs(diff)<atan2(.28,d)&&d<nearest&&ray(x,y,angle)>d-.2){nearest=d;target=&b;}}if(target){target->hp-=50;if(target->hp<=0)kills++;}won=kills==int(bots.size());}
 void tick(double dt){shot=std::max(0.,shot-dt);if(reload>0){reload-=dt;if(reload<=0)ammo=12;}if(hp<=0||won)return;for(auto& b:bots){if(b.hp<=0)continue;b.cooldown-=dt;double d=hypot(x-b.x,y-b.y),ang=atan2(y-b.y,x-b.x);if(d>.8&&ray(b.x,b.y,ang)>d-.2){double nx=b.x+cos(ang)*dt*.7,ny=b.y+sin(ang)*dt*.7;if(!solid(nx,ny)){b.x=nx;b.y=ny;}}if(d<9&&ray(b.x,b.y,ang)>d-.2&&b.cooldown<=0){hp=std::max(0,hp-8);b.cooldown=1.4;}}}
};
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
constexpr int W=960,H=600;
Game game; bool keys[256]={}; bool mouse=false; std::vector<unsigned> pixels(W*H); std::vector<double> depth(W);
void rect(int x,int y,int w,int h,unsigned c){for(int yy=std::max(0,y);yy<std::min(H,y+h);yy++)for(int xx=std::max(0,x);xx<std::min(W,x+w);xx++)pixels[yy*W+xx]=c;}
void render(){
 for(int y=0;y<H;y++){unsigned c=y<H/2?0x182534:0x303438;for(int x=0;x<W;x++)pixels[y*W+x]=c;}
 for(int col=0;col<W;col++){double angle=game.a+atan((double(col)/W*2-1)*.7);double d=game.ray(game.x,game.y,angle)*cos(angle-game.a);depth[col]=d;int h=int(H/std::max(.1,d));int shade=int(190/(1+d*.13));unsigned c=(shade<<16)|((shade*9/10)<<8)|(shade*3/4);rect(col,(H-h)/2,1,h,c);}
 std::vector<Bot> sorted=game.bots;std::sort(sorted.begin(),sorted.end(),[](Bot b,Bot c){return hypot(b.x-game.x,b.y-game.y)>hypot(c.x-game.x,c.y-game.y);});
 for(auto b:sorted){if(b.hp<=0)continue;double d=hypot(b.x-game.x,b.y-game.y),ang=atan2(b.y-game.y,b.x-game.x)-game.a;ang=atan2(sin(ang),cos(ang));if(std::abs(ang)>1.1||d<.2)continue;double z=d*cos(ang);int size=int(H/z),cx=int(W/2+tan(ang)*W/1.4);for(int xx=cx-size/5;xx<cx+size/5;xx++)if(xx>=0&&xx<W&&z<depth[xx]){rect(xx,H/2-size/3,1,size*5/6,0xBE573E);rect(xx,H/2-size/2,1,size/5,0xE3B486);}}
 rect(W/2-1,H/2-9,2,18,0xA9F7DA);rect(W/2-9,H/2-1,18,2,0xA9F7DA);
 rect(530,450,95,150,0x202930);rect(560,390,33,170,0x414C56);rect(566,379,21,25,0x161C22);if(game.shot>.14){rect(550,350,52,30,0xFFD577);}
 for(int y=0;y<16;y++){for(int x=0;x<16;x++){rect(18+x*6,18+y*6,5,5,world[y][x]=='#'?0x8596A0:0x182028);}}
 rect(18+int(game.x*6)-2,18+int(game.y*6)-2,4,4,0x57F0B2);
}
void label(HDC dc,int x,int y,const std::string& s){TextOutA(dc,x,y,s.c_str(),int(s.size()));}
LRESULT CALLBACK proc(HWND win,UINT msg,WPARAM w,LPARAM l){switch(msg){case WM_DESTROY:PostQuitMessage(0);return 0;case WM_KILLFOCUS:for(bool& k:keys)k=false;mouse=false;return 0;case WM_KEYDOWN:if(w<256)keys[w]=true;if(w==VK_ESCAPE)DestroyWindow(win);if(w=='R'&&game.reload<=0&&game.ammo<12)game.reload=1.4;if(w==VK_RETURN&&(game.hp<=0||game.won))game.reset();return 0;case WM_KEYUP:if(w<256)keys[w]=false;return 0;case WM_LBUTTONDOWN:mouse=true;return 0;case WM_LBUTTONUP:mouse=false;return 0;case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(win,&ps);render();BITMAPINFO bi={};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=W;bi.bmiHeader.biHeight=-H;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;RECT r;GetClientRect(win,&r);StretchDIBits(dc,0,0,r.right,r.bottom,0,0,W,H,pixels.data(),&bi,DIB_RGB_COLORS,SRCCOPY);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(225,245,235));label(dc,140,20,"BREACH // Training arena");label(dc,140,42,"WASD move | Arrows turn | Click / Space fire | R reload | Esc quit");label(dc,20,r.bottom-45,"Health "+std::to_string(game.hp)+"   Ammo "+std::to_string(game.ammo)+" / 12   Targets "+std::to_string(game.kills)+" / 5");if(game.reload>0)label(dc,r.right/2-40,r.bottom/2+35,"RELOADING");if(game.hp<=0||game.won)label(dc,r.right/2-140,r.bottom/2-50,game.won?"AREA CLEAR - Enter for next round":"ELIMINATED - Enter to retry");EndPaint(win,&ps);return 0;}}return DefWindowProcA(win,msg,w,l);}
int WINAPI WinMain(HINSTANCE inst,HINSTANCE,LPSTR,int){WNDCLASSA wc={};wc.lpfnWndProc=proc;wc.hInstance=inst;wc.lpszClassName="BreachArena";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);RegisterClassA(&wc);HWND win=CreateWindowA(wc.lpszClassName,"Breach - Original tactical shooter",WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,W+16,H+39,nullptr,nullptr,inst,nullptr);if(!win)return 1;LARGE_INTEGER freq,last,now;QueryPerformanceFrequency(&freq);QueryPerformanceCounter(&last);MSG msg={};while(true){while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){if(msg.message==WM_QUIT)return 0;TranslateMessage(&msg);DispatchMessage(&msg);}QueryPerformanceCounter(&now);double dt=std::min(.05,double(now.QuadPart-last.QuadPart)/freq.QuadPart);last=now; if(GetForegroundWindow()==win&&game.hp>0&&!game.won){game.a+=(keys[VK_RIGHT]-keys[VK_LEFT])*dt*2;double f=(keys['W']-keys['S'])*dt*2.5,s=(keys['D']-keys['A'])*dt*2.5;game.move(cos(game.a)*f-sin(game.a)*s,sin(game.a)*f+cos(game.a)*s);if(mouse||keys[VK_SPACE])game.fire();}game.tick(dt);InvalidateRect(win,nullptr,FALSE);UpdateWindow(win);Sleep(10);}}
#else
int main(){Game g;assert(!solid(g.x,g.y));assert(solid(-1,0));double before=g.x;g.move(.1,0);assert(g.x>before);g.move(-10,0);assert(g.x>0);g.bots={{4,3,100,2}};g.x=2;g.y=3;g.a=0;g.fire();assert(g.bots[0].hp==50&&g.ammo==11);g.tick(.3);g.fire();assert(g.won&&g.kills==1);g.reset();g.bots={{4,3,100,0}};g.y=3;g.tick(.01);assert(g.hp==92);g.ammo=0;g.reload=1;g.tick(1.1);assert(g.ammo==12);g.bots={{5,5,100,0}};g.x=2;g.y=5;g.a=0;g.shot=0;g.fire();assert(g.bots[0].hp==100);puts("PASS: collision, shooting, victory, reset, enemy damage, reload, wall occlusion");}
#endif

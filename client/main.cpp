/*
    
    SMesh* mesh = new SMesh();
    SMeshBuffer* buffer = new SMeshBuffer();
    
    S3DVertex verticies[3];
    verticies[0] = S3DVertex(0,0,0 , 0,0,1 , SColor(255,255,0,0) , 0,0);
    verticies[1] = S3DVertex(0,1,0 , 0,1,1 , SColor(255,0,255,0) , 0,0);
    verticies[2] = S3DVertex(1,0,0 , 1,0,1 , SColor(255,0,0,255) , 0,0);
    
    u16 indices[6] = {0,1,2};
    buffer->append(verticies, 3, indices, 3);
    
    mesh->addMeshBuffer(buffer);
    IMeshSceneNode* mnode = smgr->addMeshSceneNode(mesh), *cnode = smgr->addCubeSceneNode(1);
    mnode->setMaterialFlag(EMF_LIGHTING, false);
    cnode->setPosition(vector3df(0,2,0));
    mnode->setPosition(vector3df(0,0,0));
    
*/

#include <irrlicht.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <cmath>
#include <time.h>
#include <niftie>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fstream>

using namespace std;
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#include "tcp.cpp"
#include "trim.cpp"

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

#define blk u16

IVideoDriver* driver;
ICameraSceneNode* camera;
ISceneManager* smgr;
ICursorControl* cc;
dimension2d<u32> SS;

SMesh* nulmesh;

ISceneNode* playernode;

vector3df eye_offset = vector3df(0,1.8,0);

vector3df gravity = vector3df();
float jump_power = 0, walk_speed = 0;

int argc;
char **argv;

std::string readFile(std::string path) {
    ifstream inf(path.c_str());
    if (!inf) return std::string();
    inf.seekg(0, inf.end);
    int len = inf.tellg();
    inf.seekg(0, inf.beg);
    char* buffer = new char[len];
    inf.read(buffer, len);
    return std::string(buffer, len);
}

std::string optfile;

std::string getOpt(std::string opt, std::string def = std::string(), bool allowfile=true) {
    range(i,1,argc) {
        std::string arg = argv[i];
        if (arg == opt) return "true";
        else {
            size_t equ = arg.find_first_of('=');
            if (equ!=std::string::npos && arg.substr(0,equ) == opt) return arg.substr(equ+1);
        }
    }
    if (allowfile) {
        size_t c = 0;
        while (true) {
            size_t nl = optfile.find_first_of('\n', c);
            std::string line = optfile.substr(c,nl);
            size_t hpos = line.find_first_of('#');
            line = line.substr(0,hpos);
            trim(line);
            if (line.length()) {
                if (line == opt) return "true";
                else {
                    size_t equ = line.find_first_of('=');
                    if (equ!=std::string::npos && line.substr(0,equ) == opt) return line.substr(equ+1);
                }
            }
            if (nl == std::string::npos) break;
            c = nl+1;
        }
    }
    return def;
}

int RD = 1;

void screenshot(const path save2) {
    driver->writeImageToFile(driver->createScreenShot(), save2); 
}

inline int div_floor(int x, int y) {
    int q = x/y;
    int r = x%y;
    if ((r!=0) && ((r<0) != (y<0))) --q;
    return q;
}

inline int mod_floor(int x, int y) {
    int r = x%y;
    if ((r!=0) && ((r<0) != (y<0))) { r += y; }
    return r;
}


inline vector3di Fv2Iv(vector3df Fv) {
    return vector3di(floor(Fv.X),floor(Fv.Y),floor(Fv.Z));
}

inline vector3df Iv2Fv(vector3di Iv) {
    return vector3df(Iv.X,Iv.Y,Iv.Z);
}

int camflash = 0;

std::string HOST = "127.0.0.1";
int PORT = 6660;

bool connected = false;

tcp_client upd_s;

int getInt(){
    int i = 0;
    std::string intdat = upd_s.Recv(4, true);
    i += intdat.at(0);
    i += intdat.at(1) * 256;
    i += intdat.at(2) * 65536;
    if (intdat.at(3) == '-') i = -i;
    return i;
}

bool connect2Server() {
    try {
        upd_s = tcp_client();
        upd_s.Conn(HOST, PORT);
        upd_s.Send("upd");
    } catch (...) {
        return false;
    }
    if (upd_s.Recv(4) != "DONE") return false;
    return true;
}

long tick = 0;

f32 fov = 1.25663706144;

#include "color.cpp"
#include "chat.cpp"

float sqrt2 = 1.41421356237;

class MyEventReceiver : public IEventReceiver {
public:
	// This is the one method that we have to implement
	bool LB, RB;
	virtual bool OnEvent(const SEvent& event)
	{
		// Remember whether each key is down or up
		if (event.EventType == irr::EET_KEY_INPUT_EVENT){
			KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
			
			if (event.KeyInput.Key==KEY_F3&&event.KeyInput.PressedDown) {
			    screenshot("screenshot.png");
			    camflash = 15;
			    writeChat("Saved screenshot as \"screenshot.png\"");
			}
			
			if (event.KeyInput.Key==KEY_OEM_4&&event.KeyInput.PressedDown) {
			    fov-=0.1;
			    camera->setFOV(fov);
			}
			
			if (event.KeyInput.Key==KEY_OEM_6&&event.KeyInput.PressedDown) {
			    fov+=0.1;
			    camera->setFOV(fov);
			}
			
	    }
        if (event.EventType == EET_MOUSE_INPUT_EVENT)
        {
            switch(event.MouseInput.Event)
            {
            case EMIE_LMOUSE_PRESSED_DOWN:
                LB = true;
                break;

            case EMIE_LMOUSE_LEFT_UP:
                LB = false;
                break;
                
            case EMIE_RMOUSE_PRESSED_DOWN:
                RB = true;
                break;

            case EMIE_RMOUSE_LEFT_UP:
                RB = false;
                break;
            }
        }
        
        if (camera){
            return camera->OnEvent(event);
        }
        
		return false;
	}

	// This is used to check whether a key is being held down
	virtual bool IsKeyDown(EKEY_CODE keyCode) const
	{
		return KeyIsDown[keyCode];
	}
	
	MyEventReceiver()
	{
		for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
			KeyIsDown[i] = false;
	    LB=false;
	    RB=false;
	}


private:
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
};

MyEventReceiver* EVTRR;

#define KD(k) EVTRR->IsKeyDown(KEY_##k)

#include "blocks.cpp"
#include "chunk.cpp"

#include "player.cpp"

Player* player;

int main(int _argc, char **_argv){
    
    argc = _argc;
    argv = _argv;
    
    EVTRR = new MyEventReceiver();
    IrrlichtDevice *device = createDevice( video::EDT_OPENGL, dimension2d<u32>(640, 480), 16, false, false, false, EVTRR);
    
    if (!device) return -1;
    
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
    cc = device->getCursorControl();
    IGUIEnvironment* guienv = device->getGUIEnvironment();
    
    driver->beginScene(true, true, SColor(255,255,255,255));
    driver->endScene();
    
    optfile = readFile(getOpt("options_file", "options.cfg", false));
    
    RD = stoi(getOpt("render_distance","4"));
    
    playernode = smgr->addEmptySceneNode();
    playernode->setPosition(vector3df(0,16,0));
    
    camera = smgr->addCameraSceneNodeFPS(NULL,stoi(getOpt("sensitivity","300")),0);
    camera->setNearValue(0.01);
    
    player = new Player(playernode);
    
    nulmesh = new SMesh();
    
    
    range(i,0,max_msgs) chat[i]=std::string();
    font = driver->getTexture("images/fonts/ascii.png");
    
    //range(x,-1,2) range(y,-1,2) range(z,-1,2) new Chunk(vector3di(x,y,z));
    
    connected = connect2Server();
    writeChat(connected?std::string("Connected to ")+HOST+std::string(":")+to_string(PORT):std::string("Unable to connect to server."));
    
    clock_t lf = clock();
    
    while(1){
        
        {
            clock_t nc = clock();
            player->tick(((float)(nc-lf))/CLOCKS_PER_SEC);
            lf = nc;
        }
        
        laggy_server_flag = mesh_builder_flag = false;
        range (i,0,chunks.size()) {
            chunks[i]->doLoad();
            if(chunks[i]->isLoaded()&&Iv2Fv(chunks[i]->pos-getChunkFromBlock(Fv2Iv(camera->getPosition()))).getLength()>1+RD*sqrt2) {
                delete chunks[i];
                chunks.erase(chunks.begin()+i--);
            }
        }
        
        if (tick % 32 == 0 && connected) {
            range(x,-RD,1+RD) range(y,-RD,1+RD) range(z,-RD,1+RD) {
                getChunk(vector3di(x,y,z)+getChunkFromBlock(Fv2Iv(camera->getPosition())),true);
            }
        
            while (upd_s.peak()) {
                std::string upd = upd_s.Recv(4);
                printf("UPD %s\n", upd.c_str());
                
                if (upd == "CHAT") {
                    char len = upd_s.Recv(1).at(0);
                    writeChat(upd_s.Recv((int)len));
                }
                
                if (upd == "DOWN") {
                    printf("Server shut down.\n");
                    return 0;
                }
                
                if (upd == "SETB") {
                    vector3di loc = vector3di();
                    loc.X = getInt();
                    loc.Y = getInt();
                    loc.Z = getInt();
                    
                    std::string blockdata = upd_s.Recv(2, true);
                    setBlockAt(loc, ((blk)blockdata.at(0)) + 256*((blk)blockdata.at(1)));
                }
                
                if (upd == "SETG") {
                    gravity = vector3df(0,((float)getInt())/256,0);
                }
                
                if (upd == "JP=?") {
                    jump_power = ((float)getInt())/256;
                }
                
                if (upd == "WS=?") {
                    walk_speed = ((float)getInt())/256;
                }
            }
        }
        
        camera->setPosition(playernode->getPosition()+eye_offset);
        
        SS = driver->getScreenSize();
        
        camera->setAspectRatio(SS.Width/(float)SS.Height);
        
        driver->beginScene(true, true, SColor(255,0,255,255));
        
        smgr->drawAll();
        //guienv->drawAll();
        
        drawChat();
        
        if (camflash) {
            driver->draw2DRectangle(SColor(camflash|(camflash*16),255,255,255), rect<s32>(0,0 , SS.Width,SS.Height));
            camflash--;
        }
        
        driver->endScene();
        if (!device->run()) break;
        
        tick++;
    }
    
    device->drop();
    
    return 0;
}

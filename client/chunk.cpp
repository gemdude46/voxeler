#define CHL_NOT_STARTED         0
#define CHL_WAITING_4_SERVER    1
#define CHL_BUILDING_MESH       2
#define CHL_DONE                3

u16 faceInd [6] = {0,1,2 , 5,4,3};
u16 faceInd2[6] = {2,1,0 , 3,4,5};

bool mesh_builder_flag, laggy_server_flag;

class Chunk;
Chunk* getChunk(vector3di at,bool gen=false,bool mustBeLoaded=true);
blk getBlockAt(vector3di at);
void setBlockAt(vector3di at, blk to);

class Chunk {
    private:
    IMesh* createMesh();
    blk blockarr[4096];
    int loadstate;
    SMeshBuffer* buffer;
    tcp_client getter;
    int bx,by,bz;
    void setupMeshBuilder(){
        buffer = new SMeshBuffer();
        bx=by=bz=0;
        loadstate = CHL_BUILDING_MESH;
    }
    bool dirty;
    public:
    vector3di pos;
    IMeshSceneNode* node;
    void updateNode();
    Chunk (vector3di p) {
        pos = p;
        loadstate = CHL_NOT_STARTED;
        node = smgr->addMeshSceneNode(nulmesh);
        node->setPosition(Iv2Fv(p)*16);
    }
    ~Chunk () {
        node->remove();
        
    }
    blk getBlock(vector3di at) {
        return getBlock(at.X,at.Y,at.Z);
    }
    blk getBlock(int x, int y, int z) {
        if (x<0||y<0||z<0||x>15||y>15||z>15) return getBlockAt((pos*16)+vector3di(x,y,z));
        return blockarr[x+y*16+z*256];
    }
    void setBlock(vector3di at, blk to) {
        setBlock(at.X,at.Y,at.Z,to);
    }
    void setBlock(int x, int y, int z, blk to) {
        blockarr[x+y*16+z*256] = to;
        dirty = true;
    }
    bool isLoaded() {
        return loadstate > CHL_WAITING_4_SERVER;
    }
    void doLoad() {
        if (loadstate == CHL_NOT_STARTED && connected) {
            if (laggy_server_flag) return;
            laggy_server_flag = true;
            getter = tcp_client();
            getter.Conn(HOST, PORT);
            getter.Send("ch@");
            getter.Send(to_string(pos.X));
            getter.Send(" ");
            getter.Send(to_string(pos.Y));
            getter.Send(" ");
            getter.Send(to_string(pos.Z));
            getter.Send(";");
            loadstate = CHL_WAITING_4_SERVER;
            return;
        }
        if (loadstate == CHL_WAITING_4_SERVER && getter.peak()) {
            char enc = getter.Recv(1).at(0);
            if (enc == '/') {
                range(i,0,4096) {
                    std::string blockdata = getter.Recv(2, true);
                    blockarr[i] = ((blk)blockdata.at(0)) + 256*((blk)blockdata.at(1));
                }
            } else if (enc == '=') {
                std::string blockdata = getter.Recv(2, true);
                blockarr[0] = ((blk)blockdata.at(0)) + 256*((blk)blockdata.at(1));
                range(i,1,4096) blockarr[i] = blockarr[0];
            } else {
                printf("Unknown encoder %c\n", enc);
            }
            getter.Close();
            setupMeshBuilder();
            return;
        }
        if (loadstate == CHL_BUILDING_MESH
            && getChunk(pos+vector3di(1,0,0))
            && getChunk(pos+vector3di(0,1,0))
            && getChunk(pos+vector3di(0,0,1))
            && getChunk(pos+vector3di(-1,0,0))
            && getChunk(pos+vector3di(0,-1,0))
            && getChunk(pos+vector3di(0,0,-1))) {
            
            if (mesh_builder_flag) return;
            if (buildMesh()) loadstate = CHL_DONE;
        }
        if (loadstate == CHL_DONE && dirty) {
            //printf("Rebuilding\n");
            setupMeshBuilder();
        }
    }
    
    bool buildMesh(){
        mesh_builder_flag = true;
        clock_t clk = clock();
        bool f = true;
        range(x,0,16) range(y,0,16) range(z,0,16) {
            if (f) {
                x=bx;
                y=by;
                z=bz-1;
                f=false;
                continue;
            }
            blk at = getBlock(x,y,z);
            if (B_type[at] == REG_TYPE) {
                if (!BT_opaque[B_type[getBlock(x+1,y,z)]]){
                    S3DVertex verticies[6];
                    verticies[0] = S3DVertex(x+1,y,z   , x,y,z , B_clr1[at] , 0,0);
                    verticies[1] = S3DVertex(x+1,y+1,z , x,y,z , B_clr1[at] , 0,0);
                    verticies[2] = S3DVertex(x+1,y,z+1 , x,y,z , B_clr1[at] , 0,0);
                    
                    verticies[3] = S3DVertex(x+1,y+1,z+1 , x,y,z , B_clr2[at] , 0,0);
                    verticies[4] = S3DVertex(x+1,y+1,z   , x,y,z , B_clr2[at] , 0,0);
                    verticies[5] = S3DVertex(x+1,y,z+1   , x,y,z , B_clr2[at] , 0,0);
                    buffer->append(verticies, 6, faceInd, 6);
                }
                
                if (!BT_opaque[B_type[getBlock(x,y+1,z)]]){
                    S3DVertex verticies[6];
                    verticies[0] = S3DVertex(x,y+1,z   , x,y,z , B_clr1[at] , 0,0);
                    verticies[1] = S3DVertex(x,y+1,z+1 , x,y,z , B_clr1[at] , 0,0);
                    verticies[2] = S3DVertex(x+1,y+1,z , x,y,z , B_clr1[at] , 0,0);
                    
                    verticies[3] = S3DVertex(x+1,y+1,z+1 , x,y,z , B_clr2[at] , 0,0);
                    verticies[4] = S3DVertex(x,y+1,z+1   , x,y,z , B_clr2[at] , 0,0);
                    verticies[5] = S3DVertex(x+1,y+1,z   , x,y,z , B_clr2[at] , 0,0);
                    buffer->append(verticies, 6, faceInd, 6);
                }
                
                if (!BT_opaque[B_type[getBlock(x,y,z+1)]]){
                    S3DVertex verticies[6];
                    verticies[0] = S3DVertex(x,y,z+1   , x,y,z , B_clr1[at] , 0,0);
                    verticies[1] = S3DVertex(x+1,y,z+1 , x,y,z , B_clr1[at] , 0,0);
                    verticies[2] = S3DVertex(x,y+1,z+1 , x,y,z , B_clr1[at] , 0,0);
                    
                    verticies[3] = S3DVertex(x+1,y+1,z+1 , x,y,z , B_clr2[at] , 0,0);
                    verticies[4] = S3DVertex(x+1,y,z+1   , x,y,z , B_clr2[at] , 0,0);
                    verticies[5] = S3DVertex(x,y+1,z+1   , x,y,z , B_clr2[at] , 0,0);
                    buffer->append(verticies, 6, faceInd, 6);
                }
                
                if (!BT_opaque[B_type[getBlock(x-1,y,z)]]){
                    S3DVertex verticies[6];
                    verticies[0] = S3DVertex(x,y,z   , x,y,z , B_clr1[at] , 0,0);
                    verticies[1] = S3DVertex(x,y+1,z , x,y,z , B_clr1[at] , 0,0);
                    verticies[2] = S3DVertex(x,y,z+1 , x,y,z , B_clr1[at] , 0,0);
                    
                    verticies[3] = S3DVertex(x,y+1,z+1 , x,y,z , B_clr2[at] , 0,0);
                    verticies[4] = S3DVertex(x,y+1,z   , x,y,z , B_clr2[at] , 0,0);
                    verticies[5] = S3DVertex(x,y,z+1   , x,y,z , B_clr2[at] , 0,0);
                    buffer->append(verticies, 6, faceInd2, 6);
                }
                
                if (!BT_opaque[B_type[getBlock(x,y-1,z)]]){
                    S3DVertex verticies[6];
                    verticies[0] = S3DVertex(x,y,z   , x,y,z , B_clr1[at] , 0,0);
                    verticies[1] = S3DVertex(x,y,z+1 , x,y,z , B_clr1[at] , 0,0);
                    verticies[2] = S3DVertex(x+1,y,z , x,y,z , B_clr1[at] , 0,0);
                    
                    verticies[3] = S3DVertex(x+1,y,z+1 , x,y,z , B_clr2[at] , 0,0);
                    verticies[4] = S3DVertex(x,y,z+1   , x,y,z , B_clr2[at] , 0,0);
                    verticies[5] = S3DVertex(x+1,y,z   , x,y,z , B_clr2[at] , 0,0);
                    buffer->append(verticies, 6, faceInd2, 6);
                }
                
                if (!BT_opaque[B_type[getBlock(x,y,z-1)]]){
                    S3DVertex verticies[6];
                    verticies[0] = S3DVertex(x,y,z   , x,y,z , B_clr1[at] , 0,0);
                    verticies[1] = S3DVertex(x+1,y,z , x,y,z , B_clr1[at] , 0,0);
                    verticies[2] = S3DVertex(x,y+1,z , x,y,z , B_clr1[at] , 0,0);
                    
                    verticies[3] = S3DVertex(x+1,y+1,z , x,y,z , B_clr2[at] , 0,0);
                    verticies[4] = S3DVertex(x+1,y,z   , x,y,z , B_clr2[at] , 0,0);
                    verticies[5] = S3DVertex(x,y+1,z   , x,y,z , B_clr2[at] , 0,0);
                    buffer->append(verticies, 6, faceInd2, 6);
                }
            }
            if (z==15&&(((float)(clock()-clk))/CLOCKS_PER_SEC>0.048)) {
                bx=x; by=y; bz=z;
                return false;
            }
        }
        
        buffer->recalculateBoundingBox();
        SMesh* mesh = new SMesh();
        mesh->addMeshBuffer(buffer);
        mesh->recalculateBoundingBox();
        node->setMesh(mesh);
        mesh->drop();
        buffer->drop();
        node->setMaterialFlag(EMF_LIGHTING, false);
        
        dirty = false;
        return true;
    }
};

vector<Chunk*> chunks;

Chunk* getChunk(vector3di at, bool gen, bool mustBeLoaded) {
    range (i,0,chunks.size()) {
        if (at == chunks[i]->pos) {
            if (chunks[i]->isLoaded())
                return chunks[i];
            else
                return NULL;
        }
    }
    if (gen) {
        Chunk* newchunk = new Chunk(at);
        chunks.push_back(newchunk);
        return mustBeLoaded?NULL:newchunk;
    } else {
        return NULL;
    }
}

inline vector3di getChunkFromBlock(vector3di bp) {
    return vector3di(div_floor(bp.X,16),div_floor(bp.Y,16),div_floor(bp.Z,16));
}

blk getBlockAt(vector3di at) {
    Chunk* ch = getChunk(getChunkFromBlock(at));
    return ch?ch->getBlock(mod_floor(at.X,16),mod_floor(at.Y,16),mod_floor(at.Z,16)):0;
}

void setBlockAt(vector3di at, blk to) {
    Chunk* ch = getChunk(getChunkFromBlock(at));
    if (ch) ch->setBlock(mod_floor(at.X,16),mod_floor(at.Y,16),mod_floor(at.Z,16),to);
}

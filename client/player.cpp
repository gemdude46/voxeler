class Player {
    ISceneNode* node;
    public:
    vector3df position, velocity;
    Player(ISceneNode* pn) {
        node = pn;
        position = pn->getPosition();
        velocity = vector3df();
    }
    void tick(float t_t) {
        if (getChunk(getChunkFromBlock(Fv2Iv(position)))) {
            velocity += gravity * t_t;
            position += velocity * t_t;
            
            if (velocity.X > 0 &&
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(0.4,0.1, 0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(0.4,0.1,-0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(0.4,1  , 0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(0.4,1  ,-0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(0.4,1.9, 0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(0.4,1.9,-0.35)))]])) {
                position.X -= velocity.X * t_t;
                velocity.X = 0;
            }
            if (velocity.X < 0 &&
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.4,0.1, 0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.4,0.1,-0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.4,1  , 0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.4,1  ,-0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.4,1.9, 0.35)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.4,1.9,-0.35)))]])) {
                position.X -= velocity.X * t_t;
                velocity.X = 0;
            }
            if (velocity.Z > 0 &&
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df( 0.35,0.1,0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.35,0.1,0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df( 0.35,1  ,0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.35,1  ,0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df( 0.35,1.9,0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.35,1.9,0.4)))]])) {
                position.Z -= velocity.Z * t_t;
                velocity.Z = 0;
            }
            if (velocity.Z < 0 &&
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df( 0.35,0.1,-0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.35,0.1,-0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df( 0.35,1  ,-0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.35,1  ,-0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df( 0.35,1.9,-0.4)))]])||
               (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.35,1.9,-0.4)))]])) {
                position.Z -= velocity.Z * t_t;
                velocity.Z = 0;
            }
            
            bool onground = (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df( 0.35,0, 0.35)))]])||
                            (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df( 0.35,0,-0.35)))]])||
                            (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.35,0, 0.35)))]])||
                            (!BT_noclip[B_type[getBlockAt(Fv2Iv(position+vector3df(-0.35,0,-0.35)))]]);
            
            vector3df mvel = vector3df();
            
            if (KD(KEY_W)) {
                mvel.X += sin((PI/180)*camera->getRotation().Y)*walk_speed;
                mvel.Z += cos((PI/180)*camera->getRotation().Y)*walk_speed;
            }
            
            if (KD(KEY_S)) {
                mvel.X -= sin((PI/180)*camera->getRotation().Y)*walk_speed;
                mvel.Z -= cos((PI/180)*camera->getRotation().Y)*walk_speed;
            }
            
            if (KD(KEY_A)) {
                mvel.X += sin((PI/180)*(camera->getRotation().Y-90))*walk_speed;
                mvel.Z += cos((PI/180)*(camera->getRotation().Y-90))*walk_speed;
            }
            
            if (KD(KEY_D)) {
                mvel.X += sin((PI/180)*(90+camera->getRotation().Y))*walk_speed;
                mvel.Z += cos((PI/180)*(90+camera->getRotation().Y))*walk_speed;
            }
            
            if (onground && velocity.Y <= 0) {
                position.Y -= velocity.Y * t_t;
                velocity.Y = KD(SPACE)?jump_power:0;
                
                velocity.X = mvel.X;
                velocity.Z = mvel.Z;
            } else {
                mvel *= 2;
                velocity -= mvel;
                velocity *= vector3df(pow(fric.X, t_t),pow(fric.Y, t_t),pow(fric.Z, t_t));
                velocity += mvel;
            }
            
            
        }
        node->setPosition(position);
    }
};

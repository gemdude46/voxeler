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
            if (!BT_noclip[B_type[getBlockAt(Fv2Iv(position))]]) {
                position.Y -= velocity.Y * t_t;
                velocity.Y = KD(SPACE)?jump_power:0;
                
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
                    mvel.X += cos((PI/180)*camera->getRotation().Y)*walk_speed;
                    mvel.Z += sin((PI/180)*camera->getRotation().Y)*walk_speed;
                }
                
                if (KD(KEY_D)) {
                    mvel.X -= cos((PI/180)*camera->getRotation().Y)*walk_speed;
                    mvel.Z -= sin((PI/180)*camera->getRotation().Y)*walk_speed;
                }
                
                velocity.X = mvel.X;
                velocity.Z = mvel.Z;
            }
        }
        node->setPosition(position);
    }
};

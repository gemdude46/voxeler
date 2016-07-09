import random, noise

cross_seed = random.randint(-2000000000, 2000000000)+0.5

def generate_world_normal(ch):
    for i in range(16):
        for j in range(16):
            h = noise.pnoise3(i/4096.+ch.pos[0]/256.,j/4096.+ch.pos[2]/256.,cross_seed,3)*1024
            #if i == j and i == 0: print h
            for A in range(16):
                a = A+ch.pos[1]*16
                if h > 2:
                    if a < h-5: ch.setBlock((i,A,j), 1)
                    elif a < h: ch.setBlock((i,A,j), 2)
                    elif a<1+h: ch.setBlock((i,A,j), 3)
                else:
                    if a < h-5: ch.setBlock((i,A,j), 1)
                    elif a<1+h: ch.setBlock((i,A,j), 4)

def generate_world_layers(ch):
    for i in range(16):
        for j in range(16):
            ch.setBlock((i,15,j),3)
            ch.setBlock((i,14,j),2)
            ch.setBlock((i,13,j),1)

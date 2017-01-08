import random, noise

seed = random.randint(-9999999, 9999999)

def generate_world_normal(ch):
    cross_seed = seed + 0.5
    for i in range(16):
        for j in range(16):
            h = noise.pnoise3(i/1024.+ch.pos[0]/64.,j/1024.+ch.pos[2]/64.,cross_seed,3)*360
            for A in range(16):
                a = A+ch.pos[1]*16
                if h > 1:
                    if noise.snoise4(
                        i/64. + ch.pos[0]/4.,
                        a/64.,
                        j/64. + ch.pos[2]/4.,
                        cross_seed
                    ) < 0.6:
                        if a < h-5: ch.setBlock((i,A,j), 1)
                        elif a < h: ch.setBlock((i,A,j), 2)
                        elif a<1+h: ch.setBlock((i,A,j), 3)
                else:
                    if a < h-5: ch.setBlock((i,A,j), 1)
                    elif a<1+h: ch.setBlock((i,A,j), 4)
                    elif a < 0: ch.setBlock((i,A,j), 5)

def generate_world_layers(ch):
    for i in range(16):
        for j in range(16):
            ch.setBlock((i,15,j),3)
            ch.setBlock((i,14,j),2)
            ch.setBlock((i,13,j),1)

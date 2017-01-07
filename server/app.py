import socket, os, atexit, random

S_2B_inited = []
S_2B_setup = []
update_streams = []
chunk_getters = []

global_updates = []

@atexit.register
def server_closed():
    for s in update_streams:
        s.sendall('DOWN')
        s.soc.close()

def randomBetween(start, stop):
    return random.random() * (stop - start) + start

def broadcast(msg):
    global_updates.append('CHAT')
    global_updates.append(chr(len(msg)))
    global_updates.append(msg)

pong = '''
+---[2]------------------------------ PONG ------------------------------[7]---+
|                                                                              |
|                                                                              |
|                                                                              |
|                                                                              |
|                                             o                                |
|                                                                             #|
|                                                                             #|
|                                                                             #|
|                                                                             #|
|                                                                             #|
|                                                                              |
|                                                                              |
|                                                                              |
|                                                                              |
|                                                                              |
|                                                                              |
|#                                                                             |
|#                                                                             |
|#                                                                             |
|#                                                                             |
|#                                                                             |
|                                                                              |
|                                                                              |
+------------------------------------------------------------------------------+
'''

class Player:
    def __init__(self, soc):
        self.soc = soc
        self.name = '@'
        self.pos = [0,5,0]
    
    def send(self,*a,**k):
        return self.soc.send(*a,**k)
    
    def sendall(self,*a,**k):
        return self.soc.sendall(*a,**k)
    
    def recv(self,*a,**k):
        return self.soc.recv(*a,**k)
    
    def setblocking(self,*a,**k):
        return self.soc.setblocking(*a,**k)

def GEN_FLAT(chunk):
    if chunk.pos[1] < 0: chunk.blocks = [3]*4096
    else: chunk.blocks = [0]*4096

generator=GEN_FLAT

def allequal(arr):
    for i in arr:
        if not i == arr[0]:
            return False
    return True

def apv(v):
    a = abs(v)
    global_updates.append(chr(a&255)+chr((a&65535)//256)+chr(a//65535)+(('-','+')[v==a]))

class Chunk:
    def __init__(self, pos):
        self.pos = pos
        self.blocks = [0]*4096
        self.generated = False
        generator(self)
        self.generated = True
        
    def setBlock(self, pos, to):
        if self.blocks[pos[0]+pos[1]*16+pos[2]*256] == to:
            return
        self.blocks[pos[0]+pos[1]*16+pos[2]*256] = to
        if (self.generated):
            global_updates.append('SETB')
            apv(pos[0]+self.pos[0]*16)
            apv(pos[1]+self.pos[1]*16)
            apv(pos[2]+self.pos[2]*16)
            global_updates.append(chr(to&255)+chr(to//256))

    def sendTo(self, s):
        if allequal(self.blocks):
            s.sendall('='+chr(self.blocks[0]&255)+chr(self.blocks[0]//256))
        else:
            s.sendall('/'+(''.join([chr(i&255)+chr(i//256) for i in self.blocks])))

chunks = []

def getChunk(pos):
    for chunk in chunks:
        if tuple(pos) == chunk.pos:
            return chunk
    
    chunks.append(Chunk(tuple(pos)))
    return chunks[-1]

def setBlock(at, to):
    cp = (at[0]//16, at[1]//16, at[2]//16)
    bp = (at[0]% 16, at[1]% 16, at[2]% 16)
    getChunk(cp).setBlock(bp, to)

def spawnParticle(pos, vel, col, size, lifespan):
    global_updates.append('PART')
    
    for v in pos + vel: apv(int(v*256))
    for v in col: apv(v)
    apv(int(size*256))
    apv(int(lifespan*32))

def createParticleCloud(pos, scale, vel, col, min_size, max_size, min_ls, max_ls, count):
    for i in xrange(count):
        x = pos[0] + scale[0] * randomBetween(-0.5, 0.5)
        y = pos[1] + scale[1] * randomBetween(-0.5, 0.5)
        z = pos[2] + scale[2] * randomBetween(-0.5, 0.5)
        
        v = (
            randomBetween(-vel, vel),
            randomBetween(-vel, vel),
            randomBetween(-vel, vel),
        )
        
        spawnParticle((x,y,z), v, col, randomBetween(min_size, max_size), randomBetween(min_ls, max_ls))

HOST = ''
PORT = 6660

for i in [str(j) for j in range(10)]:
    for dp, _, fn in os.walk(os.path.join('scripts', i)):
        for f in fn:
            if f.endswith('.py'):
                fo = open(os.path.join(dp, f))
                exec(fo.read())
                fo.close()

getChunk((0,0,0))

def setGravity(gv):
    global gravity
    gravity = gv
    global_updates.append('SETG')
    apv(int(gv*256))

def sendSetup(s):
    s.sendall('SETG')
    apv(int(gravity*256))
    s.sendall(global_updates.pop())
    s.sendall('JP=?')
    apv(1900);
    s.sendall(global_updates.pop())
    s.sendall('WS=?')
    apv(1280);
    s.sendall(global_updates.pop())
    s.sendall('MOV2')
    apv(0);
    s.sendall(global_updates.pop())
    apv(5);
    s.sendall(global_updates.pop())
    apv(0);
    s.sendall(global_updates.pop())
    
    createParticleCloud((0,6,0), (1,2,1), 2, (255,255,255), 0.05, 0.2, 0.3, 1.5, 32)

listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

listener.bind((HOST,PORT))
listener.listen(5)

listener.setblocking(False)

while 1:
    try:
        c, a = listener.accept()
        c.setblocking(False)
        S_2B_inited.append(c)
    except socket.error: pass
    
    i = 0
    while i < len(S_2B_inited):
        s = S_2B_inited[i]
        try:
            foo = s.recv(3)
            S_2B_setup.append((s, foo))
            del S_2B_inited[i]
            i-=1
        except socket.error: pass
        i+=1
    
    for s, t in S_2B_setup:
        try:
            if t == 'png':
                s.sendall(pong)
                s.shutdown(socket.SHUT_RDWR)
            if t == 'upd':
                s.sendall('DONE')
                sendSetup(s)
                update_streams.append(Player(s))
            if t == 'ch@':
                chunk_getters.append([s,''])
        except socket.error: pass
    
    for s in update_streams:
        try:
            dat = s.recv(1)
            if dat == 'b':
                dat = ''
                s.setblocking(True)
                while dat.count(';') < 3:
                    dat += s.recv(1)
                s.setblocking(False)
                bp = [int(j) for j in dat.split(';')[:3]]
                setBlock(bp, 0)
            if dat == '@':
                dat = ''
                s.setblocking(True)
                while dat.count(';') < 3:
                    dat += s.recv(1)
                s.setblocking(False)
                s.pos = [float(j) for j in dat.split(';')[:3]]
                spawnParticle(s.pos, [0,0,0], (170,170,170), 0.1, 1)
        except socket.error: pass
    
    i = 0
    while i < len(update_streams):
        s = update_streams[i]
        try:
            s.sendall(''.join(global_updates))
        except socket.error, e:
            if e.errno == 32:
                del update_streams[i]
                i-=1
        i+=1
    
    i = 0
    while i < len(chunk_getters):
        g = chunk_getters[i]
        try:
            g[1]+=g[0].recv(32)
        except socket.error: pass
        if ';' in g[1]:
            pos = [int(j) for j in g[1][:-1].split()]
            getChunk(pos).sendTo(g[0])
            g[0].shutdown(socket.SHUT_RDWR)
            del chunk_getters[i]
            i-=1
        i+=1
    
    global_updates = []
    S_2B_setup = []

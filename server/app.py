import socket, os, atexit

S_2B_inited = []
S_2B_setup = []
update_streams = []
chunk_getters = []

global_updates = []

@atexit.register
def server_closed():
    for s in update_streams:
        s.sendall('DOWN')
        s.close()

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
    gravity = gv
    global_updates.append('SETG')
    apv(int(gv*256))

def sendSetup(s):
    s.sendall('SETG')
    apv(int(gravity*256))
    s.sendall(global_updates.pop())
    s.sendall('JP=?')
    apv(1152);
    s.sendall(global_updates.pop())
    s.sendall('WS=?')
    apv(1024);
    s.sendall(global_updates.pop())

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
                update_streams.append(s)
            if t == 'ch@':
                chunk_getters.append([s,''])
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

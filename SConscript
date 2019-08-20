from building import *

cwd     = GetCurrentDir()
CPPPATH = [cwd, cwd + '/c', str(Dir('#'))]

src     = Glob('c/*.c')


group = DefineGroup('upacker', src, depend = [''], CPPPATH = CPPPATH)

Return('group')


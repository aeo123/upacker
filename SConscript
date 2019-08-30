from building import *

cwd     = GetCurrentDir()
CPPPATH = [cwd, cwd + '/c', str(Dir('#'))]

src     = Glob('c/*.c')


group = DefineGroup('upacker', src, depend = ['PKG_USING_UPACKER'], CPPPATH = CPPPATH)

Return('group')


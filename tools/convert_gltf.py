"""Convert pinned CC0 glTF sample GLBs into compact textured runtime meshes.
Requires Python, NumPy and Pillow. Download sources separately; no network code.
"""
import io,json,struct,sys
from pathlib import Path
import numpy as np
from PIL import Image

def convert(source,destination):
    data=Path(source).read_bytes();assert data[:4]==b'glTF'
    size=struct.unpack_from('<I',data,12)[0];doc=json.loads(data[20:20+size]);offset=20+size
    binary=data[offset+8:offset+8+struct.unpack_from('<I',data,offset)[0]]
    def accessor(i):
        a=doc['accessors'][i];v=doc['bufferViews'][a['bufferView']]
        dtype={5126:'<f4',5125:'<u4',5123:'<u2',5121:'u1'}[a['componentType']]
        count={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4}[a['type']]
        step=v.get('byteStride',np.dtype(dtype).itemsize*count)
        return np.ndarray((a['count'],count),dtype=dtype,buffer=binary,offset=v.get('byteOffset',0)+a.get('byteOffset',0),strides=(step,np.dtype(dtype).itemsize)).copy()
    images=[]
    for material in doc['materials']:
        tex=material.get('pbrMetallicRoughness',{}).get('baseColorTexture',{}).get('index')
        if tex is None:im=Image.new('RGB',(1,1),(200,200,200))
        else:
            image=doc['images'][doc['textures'][tex]['source']];v=doc['bufferViews'][image['bufferView']]
            im=Image.open(io.BytesIO(binary[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']])).convert('RGB').resize((256,256),Image.Resampling.LANCZOS)
        images.append(im)
    vertices=[];faces=[]
    def matrix(node):
        if 'matrix' in node:return np.array(node['matrix']).reshape((4,4),order='F')
        m=np.eye(4);q=node.get('rotation',[0,0,0,1]);x,y,z,w=q
        m[:3,:3]=np.array([[1-2*y*y-2*z*z,2*x*y-2*z*w,2*x*z+2*y*w],[2*x*y+2*z*w,1-2*x*x-2*z*z,2*y*z-2*x*w],[2*x*z-2*y*w,2*y*z+2*x*w,1-2*x*x-2*y*y]])@np.diag(node.get('scale',[1,1,1]))
        m[:3,3]=node.get('translation',[0,0,0]);return m
    def visit(i,parent):
        node=doc['nodes'][i];m=parent@matrix(node)
        if 'mesh' in node:
            for p in doc['meshes'][node['mesh']]['primitives']:
                assert p.get('mode',4)==4
                pos=accessor(p['attributes']['POSITION']);uv=accessor(p['attributes']['TEXCOORD_0']);indices=accessor(p['indices']).flatten().reshape((-1,3))
                pos=(m@np.column_stack((pos,np.ones(len(pos)))).T).T[:,:3];pos=np.column_stack((pos[:,0],-pos[:,2],pos[:,1]));base=len(vertices)
                vertices.extend(np.column_stack((pos,uv)).tolist());faces.extend([(int(a)+base,int(b)+base,int(c)+base,p.get('material',0)) for a,b,c in indices])
        for child in node.get('children',[]):visit(child,m)
    for root in doc['scenes'][doc.get('scene',0)]['nodes']:visit(root,np.eye(4))
    v=np.array(vertices);low=v[:,:3].min(axis=0);high=v[:,:3].max(axis=0);height=high[2]-low[2];v[:,0]-=(low[0]+high[0])/2;v[:,1]-=(low[1]+high[1])/2;v[:,2]-=low[2];v[:,:3]/=height
    with open(destination,'wb') as f:
        f.write(b'BRM1'+struct.pack('<III',len(images),len(v),len(faces)))
        for im in images:
            f.write(struct.pack('<II',im.width,im.height));a=np.array(im,dtype=np.uint32);pixels=(a[:,:,0]<<16)|(a[:,:,1]<<8)|a[:,:,2];f.write(pixels.astype('<u4').tobytes())
        f.write(v.astype('<f4').tobytes());f.write(np.array(faces,dtype='<u4').tobytes())
    print(destination,len(v),'vertices',len(faces),'triangles')
if __name__=='__main__':convert(sys.argv[1],sys.argv[2])

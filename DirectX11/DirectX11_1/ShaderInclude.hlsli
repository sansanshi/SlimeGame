
Texture2D _tex:register(t0);
Texture2D _sph:register(t1);
Texture2D _spa:register(t2);

Texture2D _normalTex:register(t5);
Texture2D _heightMap:register(t6);
Texture2D _dispMap:register(t7);
Texture2D _dispMask:register(t8);
Texture2D _decalMap:register(t9);//デカール
Texture2D _shadowTex:register(t10);//ライトからの深度値をテクスチャとして受け取る
Texture2D _cameraDepthTex:register(t12);
Texture2D _subTex:register(t13);//サブテクスチャ
SamplerState _samplerState:register(s0);
SamplerState _samplerStateDisp:register(s1);
SamplerState _samplerState_clamp:register(s2);

#pragma once
class Scene
{
public:
	Scene();
	~Scene();

	virtual void Update(int mouseWheelCnt) = 0;
};


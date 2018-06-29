#pragma once

class IPoint
{
public:
	int x;
	int y;

	IPoint(int X, int Y){ x = X; y = Y; }
	IPoint(){ x = 0; y = 0; }
	int GetWidth(){ return x; }
	int GetHeight(){ return y; }
};

class IRect
{
public:
	int x;
	int y;
	int width;
	int height;
	IRect(int X, int Y, int Width, int Height){ x = X; y = Y; width = Width; height = Height; }
	IRect(){ x = 0; y = 0; width = 0; height = 0; }
};
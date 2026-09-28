#pragma once
#include "DrumEngine.h"
#include <vector>
struct Ev { int t; int inst; float vel; };
// render with events at exact samples and additional forced segment boundaries every B samples
void renderSeg(oildrum::DrumEngine& e, const std::vector<Ev>& ev, int N, int B, std::vector<float>& L, std::vector<float>& R);

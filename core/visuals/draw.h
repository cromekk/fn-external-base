#pragma once

void draw2DBox( float BoxWidth,Vector2 Root,Vector2 HeadBox,ImColor Color )
{
	ImGui::GetBackgroundDrawList( )->AddRect( ImVec2( HeadBox.x - BoxWidth / 2.0f,HeadBox.y ),ImVec2( Root.x + BoxWidth / 2.0f,Root.y ),Color,0.0f,0,1.0f );
	ImGui::GetBackgroundDrawList( )->AddRectFilled( ImVec2( HeadBox.x - BoxWidth / 2.0f,HeadBox.y ),ImVec2( Root.x + BoxWidth / 2.0f,Root.y ),ImColor( 51,51,51,90 ),1.0f );
}

void drawLine(Vector2 target, const ImColor color)
{
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(settings.ScreenCenterX, settings.ScreenHeight), ImVec2(target.x, target.y), color, 0.1f);
}

void drawDistance(Vector2 location, float distance, const ImColor color)
{
	char dist[64];
	sprintf_s(dist, "%.fm", distance);
	ImVec2 text_size = ImGui::CalcTextSize(dist);
	ImGui::GetForegroundDrawList()->AddText(ImVec2(location.x - text_size.x / 2, location.y - text_size.y / 2), color, dist);
}

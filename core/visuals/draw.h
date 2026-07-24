#pragma once

void draw2DBox( float BoxWidth,Vector2 Root,Vector2 HeadBox,ImColor Color )
{
	ImGui::GetBackgroundDrawList( )->AddRect( ImVec2( HeadBox.x - BoxWidth / 2.0f,HeadBox.y ),ImVec2( Root.x + BoxWidth / 2.0f,Root.y ),Color,0.0f,0,1.0f );
	ImGui::GetBackgroundDrawList( )->AddRectFilled( ImVec2( HeadBox.x - BoxWidth / 2.0f,HeadBox.y ),ImVec2( Root.x + BoxWidth / 2.0f,Root.y ),ImColor( 51,51,51,90 ),1.0f );
}
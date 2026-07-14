#pragma once

void renderMenu()
{
	if (aim.showFov) ImGui::GetForegroundDrawList()->AddCircle(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), aim.fovValue, ImColor(250, 250, 250, 250), 100, 1.0f);

	if (GetAsyncKeyState(VK_INSERT) & 1) menu.showMenu = !menu.showMenu;
	if ( menu.showMenu )
	{
		ImGui::SetNextWindowSize({ 620, 350 });
		ImGui::Begin("Fortnite", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
		if (ImGui::Button("Aimbot", { 196, 20 })) menu.tab = 0;
		ImGui::SameLine();
		if (ImGui::Button("Visuals", { 196, 20 })) menu.tab = 1;
		ImGui::SameLine();
		if (ImGui::Button("Misc", { 196, 20 })) menu.tab = 2;
		switch ( menu.tab )
		{
			case 0:
			{
				ImGui::Checkbox("Show Fov", &aim.showFov);
				ImGui::SliderFloat("##Fov", &aim.fovValue, 50, 300, "Fov: %.2f");
				break;
			}
			case 1:
			{
				ImGui::Checkbox("Enable", &visual.enable);

				ImGui::Checkbox("Box", &visual.box);
				ImGui::SameLine();
				ImGui::Checkbox("Fill Box", &visual.fillBox);

				ImGui::Checkbox("Line", &visual.line);

				ImGui::Checkbox("Distance", &visual.distance);
				break;
			}
			case 2:
			{
				if (ImGui::Button("Unload", { 120, 20 })) exit(0);
				break;
			}
		}
		ImGui::End();
	}
}

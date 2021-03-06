#include "RenderOrderWindow.hpp"

#include <imgui.h>

#include <cassert>


RenderOrderWindow::RenderOrderWindow(std::string_view windowName, ApplicationState* appState)
	: Window(windowName, false)
	, m_appState(appState)
{
	assert(m_appState);
	if (m_appState)
	{
		setScope(&m_appState->rootOperator);
	}
}

void RenderOrderWindow::setScope(ZigZag::BaseOperator* rootObject)
{
	m_renderOrder = std::make_unique<ZigZag::RenderOrder>(rootObject);
}

void RenderOrderWindow::draw()
{
	m_renderOrder->construct();

	auto& order = m_renderOrder->order();

	int binCount = 0;
	for (auto& bin : order)
	{
		if (bin.empty()) break;
		++binCount;
	}

	m_appState->style.push("RenderOrderWindow");

	auto basePos = ImGui::GetCursorPos();

	for (int bin = 0; bin < binCount; ++bin)
	{
		ImGui::SetCursorPos({ basePos.x + bin * 140.0f, basePos.y });

		for (auto op : order[bin])
		{
			ImGui::Text(op->getName().c_str());
		}
	}

	m_appState->style.pop("RenderOrderWindow");
}

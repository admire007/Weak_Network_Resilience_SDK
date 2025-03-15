#include "basic_form.h"

#include "stdafx.h"
#include "ui_wnd.h"
#include "main.h"


const std::wstring BasicForm::kClassName = L"Basic";

BasicForm::BasicForm(MainThread* ui_thread) :
    ui_thread_(ui_thread)
{
}

BasicForm::~BasicForm()
{
}

std::wstring BasicForm::GetSkinFolder()
{
	return L"basic";
}

std::wstring BasicForm::GetSkinFile()
{
	return L"basic.xml";
}

std::wstring BasicForm::GetWindowClassName() const
{
	return kClassName;
}

void BasicForm::InitWindow()
{
	xrtc::XRTCEngine::Init();
	
	InitComboCam();


}

LRESULT BasicForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

void BasicForm::ShowMax() {
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
}

ui::Control* BasicForm::CreateControl(const std::wstring& pstrClass) {
	//自定义控件
	if (!_tcscmp(pstrClass.c_str(), _T("VideoWnd"))) {
		CWndUI* wnd = new CWndUI(this->m_hWnd);
		return wnd;
	}

	return nullptr;
}


bool BasicForm::Notify(ui::EventArgs* msg)
{
	    std::wstring name = msg->pSender->GetName();//事件的发送者
		if (msg->Type == ui::kEventClick) {
			if (L"btn_device_start" == name) {
				OnBtnDeviceStartClick();
			}else if (L"btn_prev" == name) {
				OnBtnPreviewClick();
			}else if(L"btn_push" == name) {
			    OnBtnPushClick();
		    }
			return true;
		}
}

void BasicForm::InitComboCam() {
	int total = xrtc::XRTCEngine::GetGameraCount();
	if (total <= 0) {
		return;
	}

	combo_cam_ = dynamic_cast<ui::Combo*>(FindControl(L"cam_combo"));
	if (!combo_cam_) {
		return;
	}

	for (int i = 0; i < total; ++i) {
		std::string device_name;
		std::string device_id;
		xrtc::XRTCEngine::GetCameraInfo(i, device_name, device_id);
		ui::ListContainerElement* element = new ui::ListContainerElement();
		element->SetClass(L"listitem");
		element->SetFixedHeight(30);
		element->SetText(nbase::UTF8ToUTF16(device_name));
		element->SetDataID(nbase::UTF8ToUTF16(device_id));
		element->SetTextPadding({ 6, 0, 6, 0 });
		combo_cam_->Add(element);
	}

	// 默认选中第一条数据
	int count = combo_cam_->GetCount();
	if (count > 0) {
		combo_cam_->SelectItem(0);
	}

	combo_cam_->AttachSelect(nbase::Bind(&BasicForm::OnComboCamItemSelected, this,
		std::placeholders::_1));
}

bool BasicForm::OnComboCamItemSelected(ui::EventArgs* msg) {
	return true;
}

void BasicForm::OnBtnDeviceStartClick()
{
			btn_device_start_->SetText(L"启动音视频设备");
	
}


bool BasicForm::StartDevice() {

	return true;
}

bool BasicForm::StopDevice() {

	return true;
}

void BasicForm::OnBtnPreviewClick() {
	
	
}

bool BasicForm::StartPreview() {
	
		return false;
}

bool BasicForm::StopPreview() {

	return true;
}

void BasicForm::OnBtnPushClick() {
	
	btn_push_->SetText(L"开始推流");
}

bool BasicForm::StartPush() {
	
	return true;
}

bool BasicForm::StopPush() {

	return true;
}

//api_thread调用ShowToast   在ui_thread  调用CallOnUIThread
void BasicForm::ShowToast(const std::wstring& toast, bool err) {
	CallOnUIThread([=]() {
		ui::Label* toast_text = dynamic_cast<ui::Label*>(FindControl(L"toast_text"));
		if (toast_text) {
			if (err) {
				toast_text->SetStateTextColor(ui::kControlStateNormal, L"red");
			}

			toast_text->SetText(toast);
		}
	});
}

void BasicForm::CallOnUIThread(const std::function<void(void)>& task) {
	ui_thread_->message_loop()->PostTask(task);
}







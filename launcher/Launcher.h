#pragma once

#include "../sdk/sdk.h"

namespace launcher {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Launcher 摘要
	/// </summary>
	public ref class Launcher : public System::Windows::Forms::Form
	{
	public:
		Launcher(void)
		{
			InitializeComponent();
			//
			//TODO:  在此处添加构造函数代码
			//
		}

	protected:
		/// <summary>
		/// 清理所有正在使用的资源。
		/// </summary>
		~Launcher()
		{
			if (components)
			{
				delete components;
			}
		}
    private: System::Windows::Forms::Button^ Start;
    private: System::Windows::Forms::Button^ Stop;
    private: System::Windows::Forms::CheckBox^ cbDebug;
    private: System::Windows::Forms::TextBox^ tbPort;
    private: System::Windows::Forms::Label^ lPort;


    private: System::Windows::Forms::LinkLabel^ llGJH;

    protected:

    protected:


    protected:

	private:
		/// <summary>
		/// 必需的设计器变量。
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// 设计器支持所需的方法 - 不要修改
		/// 使用代码编辑器修改此方法的内容。
		/// </summary>
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(Launcher::typeid));
            this->Start = (gcnew System::Windows::Forms::Button());
            this->Stop = (gcnew System::Windows::Forms::Button());
            this->cbDebug = (gcnew System::Windows::Forms::CheckBox());
            this->tbPort = (gcnew System::Windows::Forms::TextBox());
            this->lPort = (gcnew System::Windows::Forms::Label());
            this->llGJH = (gcnew System::Windows::Forms::LinkLabel());
            this->SuspendLayout();
            // 
            // Start
            // 
            this->Start->Font = (gcnew System::Drawing::Font(L"宋体", 14.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(134)));
            this->Start->Location = System::Drawing::Point(29, 61);
            this->Start->Name = L"Start";
            this->Start->Size = System::Drawing::Size(100, 40);
            this->Start->TabIndex = 0;
            this->Start->Text = L"启动";
            this->Start->UseVisualStyleBackColor = true;
            this->Start->Click += gcnew System::EventHandler(this, &Launcher::Start_Click);
            // 
            // Stop
            // 
            this->Stop->Enabled = false;
            this->Stop->Font = (gcnew System::Drawing::Font(L"宋体", 14.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(134)));
            this->Stop->Location = System::Drawing::Point(171, 61);
            this->Stop->Name = L"Stop";
            this->Stop->Size = System::Drawing::Size(100, 40);
            this->Stop->TabIndex = 1;
            this->Stop->Text = L"关闭";
            this->Stop->UseVisualStyleBackColor = true;
            this->Stop->Click += gcnew System::EventHandler(this, &Launcher::Stop_Click);
            // 
            // cbDebug
            // 
            this->cbDebug->AutoSize = true;
            this->cbDebug->Checked = true;
            this->cbDebug->CheckState = System::Windows::Forms::CheckState::Checked;
            this->cbDebug->Location = System::Drawing::Point(190, 23);
            this->cbDebug->Name = L"cbDebug";
            this->cbDebug->Size = System::Drawing::Size(54, 16);
            this->cbDebug->TabIndex = 2;
            this->cbDebug->Text = L"Debug";
            this->cbDebug->UseVisualStyleBackColor = true;
            // 
            // tbPort
            // 
            this->tbPort->Location = System::Drawing::Point(77, 21);
            this->tbPort->Name = L"tbPort";
            this->tbPort->Size = System::Drawing::Size(52, 21);
            this->tbPort->TabIndex = 3;
            this->tbPort->Text = L"10086";
            this->tbPort->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
            // 
            // lPort
            // 
            this->lPort->AutoSize = true;
            this->lPort->Location = System::Drawing::Point(42, 25);
            this->lPort->Name = L"lPort";
            this->lPort->Size = System::Drawing::Size(29, 12);
            this->lPort->TabIndex = 4;
            this->lPort->Text = L"Port";
            // 
            // llGJH
            // 
            this->llGJH->AutoSize = true;
            this->llGJH->Location = System::Drawing::Point(47, 120);
            this->llGJH->Name = L"llGJH";
            this->llGJH->Size = System::Drawing::Size(209, 12);
            this->llGJH->TabIndex = 7;
            this->llGJH->TabStop = true;
            this->llGJH->Text = L"除非你知道你在做什么，否则不要启动";
            this->llGJH->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &Launcher::linkLabel2_LinkClicked);
            // 
            // Launcher
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(304, 141);
            this->Controls->Add(this->llGJH);
            this->Controls->Add(this->lPort);
            this->Controls->Add(this->tbPort);
            this->Controls->Add(this->cbDebug);
            this->Controls->Add(this->Stop);
            this->Controls->Add(this->Start);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
            this->MaximizeBox = false;
            this->Name = L"Launcher";
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"WeChatFerry Launcher";
            this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &Launcher::Launcher_FormClosing);
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
    private: System::Void Start_Click(System::Object^ sender, System::EventArgs^ e) {
        this->Start->Enabled = false;
        this->tbPort->Enabled = false;
        this->cbDebug->Enabled = false;
        this->Stop->Enabled = true;
        int port = int::Parse(tbPort->Text);
        bool debug = cbDebug->Checked;
        WxInitSDK(debug, port);
    }
    private: System::Void Stop_Click(System::Object^ sender, System::EventArgs^ e) {
        this->Stop->Enabled = false;
        this->Start->Enabled = true;
        this->tbPort->Enabled = true;
        this->cbDebug->Enabled = true;
        WxDestroySDK();
    }
    private: System::Void Launcher_Closing(System::Object^ sender, System::EventArgs^ e) {
        if (this->Stop->Enabled) {  // 已经启动
            WxDestroySDK();
        }
    }
    private: System::Void Launcher_FormClosing(System::Object^ sender, System::Windows::Forms::FormClosingEventArgs^ e)
    {
        if (this->Stop->Enabled) {  // 已经启动
            System::Windows::Forms::DialogResult ret;
            ret = MessageBox::Show("确定退出？", "警告", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
            if (ret == System::Windows::Forms::DialogResult::Yes) {
                WxDestroySDK();
                e->Cancel = false;
            }
            else {
                e->Cancel = true;
            }
        }
    }
    private: System::Void linkLabel2_LinkClicked(System::Object^ sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^ e) {
        this->llGJH->LinkVisited = true;
        System::Diagnostics::Process::Start("https://mp.weixin.qq.com/s/CGLfSaNDy8MyuyPWGjGJ7w");
    }
};
}

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
            this->Start = (gcnew System::Windows::Forms::Button());
            this->Stop = (gcnew System::Windows::Forms::Button());
            this->SuspendLayout();
            // 
            // Start
            // 
            this->Start->Font = (gcnew System::Drawing::Font(L"宋体", 14.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(134)));
            this->Start->Location = System::Drawing::Point(36, 39);
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
            this->Stop->Location = System::Drawing::Point(177, 39);
            this->Stop->Name = L"Stop";
            this->Stop->Size = System::Drawing::Size(100, 40);
            this->Stop->TabIndex = 1;
            this->Stop->Text = L"关闭";
            this->Stop->UseVisualStyleBackColor = true;
            this->Stop->Click += gcnew System::EventHandler(this, &Launcher::Stop_Click);
            // 
            // Launcher
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(308, 122);
            this->Controls->Add(this->Stop);
            this->Controls->Add(this->Start);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox = false;
            this->Name = L"Launcher";
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"WeChatFerry Launcher";
            this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &Launcher::Launcher_FormClosing);
            this->ResumeLayout(false);

        }
#pragma endregion
    private: System::Void Start_Click(System::Object^ sender, System::EventArgs^ e) {
        this->Start->Enabled = false;
        this->Stop->Enabled = true;
        WxInitSDK(true);
    }
    private: System::Void Stop_Click(System::Object^ sender, System::EventArgs^ e) {
        this->Stop->Enabled = false;
        this->Start->Enabled = true;
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
    };
}

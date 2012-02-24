#pragma once

namespace cliTest {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class osgWindow : public System::Windows::Forms::Form
	{
	public:
		osgWindow(void)
		{
			InitializeComponent();
		}

	protected:
		~osgWindow()
		{
			if (components)
			{
				delete components;
			}
		}

		void runThread();
		void initializeOSG();

	private:
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
			this->SuspendLayout();
			// 
			// osgWindow
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 262);
			this->Name = L"osgWindow";
			this->Text = L"osgWindow";
			this->Load += gcnew System::EventHandler(this, &osgWindow::osgWindow_Load);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void osgWindow_Load(System::Object^  sender, System::EventArgs^  e) {
				 initializeOSG();
			 }
	};
}

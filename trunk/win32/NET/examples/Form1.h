#pragma once
#include <netcdf.h>

namespace tst_netcdf
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary> 
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public __gc class Form1 : public System::Windows::Forms::Form
	{	
	public:
		Form1(void)
		{
			InitializeComponent();
		}
  
	protected:
		void Dispose(Boolean disposing)
		{
			if (disposing && components)
			{
				components->Dispose();
			}
			__super::Dispose(disposing);
		}
	private: System::Windows::Forms::Button *  button1;
	private: System::Windows::Forms::TextBox *  textBox1;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container * components;

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->button1 = new System::Windows::Forms::Button();
			this->textBox1 = new System::Windows::Forms::TextBox();
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(88, 40);
			this->button1->Name = S"button1";
			this->button1->TabIndex = 0;
			this->button1->Text = S"Push Me!";
			this->button1->Click += new System::EventHandler(this, &tst_netcdf::Form1::button1_Click);
			// 
			// textBox1
			// 
			this->textBox1->Location = System::Drawing::Point(40, 104);
			this->textBox1->Multiline = true;
			this->textBox1->Name = S"textBox1";
			this->textBox1->Size = System::Drawing::Size(224, 152);
			this->textBox1->TabIndex = 1;
			this->textBox1->Text = S"";
			// 
			// Form1
			// 
			this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
			this->ClientSize = System::Drawing::Size(292, 273);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->button1);
			this->Name = S"Form1";
			this->Text = S"Form1";
			this->ResumeLayout(false);

		}	
	private: System::Void button1_Click(System::Object *  sender, System::EventArgs *  e)
			 {
				 int ncid, varid;
				 int dimids[2];
				 int temp_data[10][5];
				 int res;

				 // phoney up some data.
				 for (int i=0; i<10; i++)
					for (int j=0; j<5; j++)
						temp_data[i][j] = 100 + i;

				 textBox1->AppendText("Creating file...");
				 if ((res = nc__create("test.nc", NC_CLOBBER, 0, NULL, &ncid)))
				 {
					 textBox1->AppendText("error creating: ");
					 textBox1->AppendText(nc_strerror(res));
					 return;
				 }
				 textBox1->AppendText("Defining dims...");
				 if ((res = nc_def_dim(ncid, "lat", 10, &(dimids[0]))))
				 {
					 textBox1->AppendText("error nc_def_dim: ");
					 textBox1->AppendText(nc_strerror(res));
					 return;
				 }
				 if ((res = nc_def_dim(ncid, "lon", 5, &dimids[1])))
				 {
					 textBox1->AppendText("error nc_def_dim: ");
					 textBox1->AppendText(nc_strerror(res));
					 return;
				 }
				 if ((res = nc_def_var(ncid, "temp", NC_INT, 2, dimids, &varid)))
				 {
					 textBox1->AppendText("error nc_def_dim: ");
					 textBox1->AppendText(nc_strerror(res));
					 return;
				 }
				 textBox1->AppendText("Ending definition...");
				 if ((res = nc_enddef(ncid)))
				 {
					 textBox1->AppendText("error nc_enddef: ");
					 textBox1->AppendText(nc_strerror(res));
					 return;
				 }
				 textBox1->AppendText("Writing data...");
				 if ((res = nc_put_var_int(ncid, varid, &temp_data[0][0])))
				 {
					 textBox1->AppendText("error nc_put_var_int: ");
					 textBox1->AppendText(nc_strerror(res));
					 return;
				 }
				 if ((res = nc_close(ncid)))
				 {
					 textBox1->AppendText("\nerror nc_close: ");
					 textBox1->AppendText(nc_strerror(res));
					 return;
				 }

			 }

	};
}



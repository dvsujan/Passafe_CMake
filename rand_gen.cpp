#include "rand_gen.h"
#include "ui_rand_gen.h"
#include<iostream>
#include<QString>
#include<QClipboard>
#include "new_entry.h"
#include<QRandomGenerator>

static int rand_len;
QString f_pass ;
QString passWord;
static int ar1 ;
static int ar2 ;
static std::string possibleCharacters;

rand_gen::rand_gen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::rand_gen)
{
    ui->setupUi(this);
    this->setWindowTitle("Generate Password");


}

rand_gen::~rand_gen()
{
     delete ui;
}


std::string str_a(){
    int a = ar1 ;
    int b = ar2;




    if(a == 2  && b == 2 ){
        possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+{}:';<.,>/?";
        return possibleCharacters;

        }
    else if(a == 2 && b == 0) {

        possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        return possibleCharacters;



        }
    else if(a == 0 && b == 2) {

        possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()_+{}:';<.,>/?";
        return possibleCharacters;



        }



    else{
        possibleCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        return possibleCharacters;

        }
}



QString GetRandomString(int length)
{

  std::string  Chars_ = str_a();
  QString qstr = QString::fromStdString(Chars_);

   const int randomStringLength = length;

   QString randomString;
   for(int i=0; i<randomStringLength; ++i)
   {
       int index = rand() % qstr.length();
       QChar nextChar = qstr.at(index);
       randomString.append(nextChar);
   }
   return randomString;
}




void rand_gen::on_pushButton_clicked()
{
    QString clip_text = ui->generated_pass->text();
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(clip_text);

}



void rand_gen::on_cancle_button_clicked()
{

    close();



}

void rand_gen::on_horizontalSlider_sliderMoved(int position)
{
    rand_len = position;
    f_pass = GetRandomString(rand_len);
    QString s = QString::number(position);
    ui->pass_len->setText(s);
    ui->generated_pass->setText(f_pass);


}


void rand_gen::on_ok_button_clicked()
{
    passWord = f_pass ;
    close();


}

void rand_gen::on_checkBox_2_stateChanged(int arg1)
{
    ar1 = arg1 ;



}

void rand_gen::on_checkBox_stateChanged(int arg1)
{

    ar2 = arg1 ;


}

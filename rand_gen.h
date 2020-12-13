#ifndef RAND_GEN_H
#define RAND_GEN_H

#include <QWidget>
#include "passman.h"
#include<QString>
#include<iostream>


namespace Ui {
class rand_gen;
}

class rand_gen : public QWidget
{
    Q_OBJECT

public:
    explicit rand_gen(QWidget *parent = 0);
    passman password_manager ;

    ~rand_gen();

protected:


private slots:


    void on_pushButton_clicked();

    void on_cancle_button_clicked();

    void on_horizontalSlider_sliderMoved(int position);

    void on_ok_button_clicked();




    void on_checkBox_2_stateChanged(int arg1);

    void on_checkBox_stateChanged(int arg1);

private:
    Ui::rand_gen *ui;
};

#endif // RAND_GEN_H

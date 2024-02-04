#include "checador.h"
#include "ui_checador.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

Checador::Checador(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Checador)
{
    ui->setupUi(this);

    showFullScreen();
    ui->input->setFocus();

    QPixmap pixmap = QPixmap(":/images/lupa.png");
    ui->imagen->setPixmap(pixmap.scaled(ui->imagen->width(), ui->imagen->height(), Qt::KeepAspectRatio));
}

Checador::~Checador()
{
    delete ui;
}

void Checador::set_to_not_found(){
    ui->nombre->setStyleSheet("color : #F4103B;");
    ui->nombre->setText("NO ENCONTRADO");
    ui->piezas2->hide();
    ui->piezas3->hide();
    ui->piezas4->hide();
    ui->clave->setText("-");
    ui->clave_alterna->setText("-");
    ui->precio1->setText("-");
    ui->precio2->hide();
    ui->precio3->hide();
    ui->precio4->hide();
    QPixmap pixmap = QPixmap(":/images/notfound.png");
    ui->imagen->setPixmap(pixmap.scaled(ui->imagen->width(), ui->imagen->height(), Qt::KeepAspectRatio));
}

void Checador::on_input_returnPressed()
{
    ui->input->selectAll();

    //CONEXION POR DEFAULT
    QString ip = "192.168.0.105";
    QString port = "3306";
    QString base = "sicar";
    QString username = "consultas";
    QString password = "123456";

    QFile file("ip_server.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        ip = in.readLine();
        port = in.readLine();
        base = in.readLine();
        username = in.readLine();
        password = in.readLine();
        file.close();
    }

    QSqlDatabase sicardb = QSqlDatabase::addDatabase("QMYSQL");
    sicardb.setHostName(ip);
    sicardb.setPort(port.toInt());
    sicardb.setDatabaseName(base);
    sicardb.setUserName(username);
    sicardb.setPassword(password);
    sicardb.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=5");

    if( !sicardb.isValid() ){
        set_to_not_found();
        ui->nombre->setStyleSheet("color : #F4103B;");
        ui->nombre->setText("NO SE PUDO CARGAR EL DRIVER");
        return;
    }

    if( !sicardb.open() ){
        set_to_not_found();
        ui->nombre->setStyleSheet("color : #F4103B;");
        ui->nombre->setText("NO SE CONECTO A LA BASE DE DATOS");
        return;
    }

    QSqlQuery consulta;

    bool precio_fijo = false;
    bool multiplicador = false;
    float modificador = 0;

    QString busqueda = ui->input->text().toUpper();
    if( busqueda.contains("/") ){
        precio_fijo = true;
        modificador = busqueda.left( busqueda.indexOf("/") ).toFloat();
        busqueda = busqueda.right( busqueda.length() - busqueda.indexOf("/")-1 );
    }
    if( busqueda.contains("-") ){
        precio_fijo = true;
        modificador = busqueda.left( busqueda.indexOf("-") ).toFloat();
        busqueda = busqueda.right( busqueda.length() - busqueda.indexOf("-")-1 );
    }
    if( busqueda.contains("*") ){
        multiplicador = true;
        modificador = busqueda.left( busqueda.indexOf("*") ).toFloat();
        busqueda = busqueda.right( busqueda.length() - busqueda.indexOf("*")-1 );
    }

    consulta.exec("SELECT * FROM articulo LEFT JOIN articuloimagen ON articuloimagen.art_id=articulo.art_id LEFT JOIN imagen ON articuloimagen.img_id=imagen.img_id LEFT JOIN articuloimpuesto ON articuloimpuesto.art_id=articulo.art_id LEFT JOIN impuesto ON impuesto.imp_id=articuloimpuesto.imp_id WHERE (clave='" + busqueda + "' OR claveAlterna='" + busqueda + "') AND articulo.status=1");

    if( consulta.next() ){

        ui->nombre->setStyleSheet("color : #004FA8;");
        ui->nombre->setText( consulta.value("descripcion").toString() );
        ui->localizacion->setText( consulta.value("localizacion").toString() );
        ui->clave->setText( consulta.value("clave").toString() );
        ui->clave_alterna->setText( consulta.value("claveAlterna").toString() );

        if( consulta.value("claveAlterna").toString().isEmpty() ){
            ui->clave_alterna->hide();
        }else{
            ui->clave_alterna->show();
        }

        QString temp;
        float impuesto = 1;
        if( !consulta.value("impuesto").toString().isEmpty() ){
            impuesto += consulta.value("impuesto").toFloat() / 100;
        }

        if(precio_fijo){
            temp.setNum( modificador, 'f', 2 );
        }else if(multiplicador){
            temp.setNum( modificador * impuesto * consulta.value("precio1").toFloat(), 'f', 2 );
        }else{
            temp.setNum( consulta.value("precio1").toFloat() * impuesto, 'f', 2 );
        }
        ui->precio1->setText("$ " + temp);

        ui->otros_precios->hide();
        if( consulta.value("precio2").toFloat() > 0 && consulta.value("mayoreo2").toFloat() > 0 ){
            temp.setNum( consulta.value("precio2").toFloat() * impuesto, 'f', 2 );
            ui->precio2->setText("$ " + temp);
            ui->precio2->show();
            temp.setNum( consulta.value("mayoreo2").toFloat(), 'f', 2 );
            ui->piezas2->setText(temp + " pz ");
            ui->piezas2->show();
            ui->otros_precios->show();
        }else{
            ui->precio2->setText("");
            ui->precio2->hide();
            ui->piezas2->setText("");
            ui->piezas2->hide();
        }

        if( consulta.value("precio3").toFloat() > 0 && consulta.value("mayoreo3").toFloat() > 0 ){
            temp.setNum( consulta.value("precio3").toFloat() * impuesto, 'f', 2 );
            ui->precio3->setText("$ " + temp);
            ui->precio3->show();
            temp.setNum( consulta.value("mayoreo3").toFloat(), 'f', 2 );
            ui->piezas3->setText(temp + " pz ");
            ui->piezas3->show();
            ui->otros_precios->show();
        }else{
            ui->precio3->setText("");
            ui->precio3->hide();
            ui->piezas3->setText("");
            ui->piezas3->hide();
        }

        if( consulta.value("precio4").toFloat() > 0 && consulta.value("mayoreo4").toFloat() > 0 ){
            temp.setNum( consulta.value("precio4").toFloat() * impuesto, 'f', 2 );
            ui->precio4->setText("$ " + temp);
            ui->precio4->show();
            temp.setNum( consulta.value("mayoreo4").toFloat(), 'f', 2 );
            ui->piezas4->setText(temp + " pz ");
            ui->piezas4->show();
            ui->otros_precios->show();
        }else{
            ui->precio4->setText("");
            ui->precio4->hide();
            ui->piezas4->setText("");
            ui->piezas4->hide();
        }

        if( !consulta.value("imagen").isNull() ){
            QByteArray byteArray = consulta.value("imagen").toByteArray();
            QImage image = QImage::fromData(byteArray);
            QPixmap pixmap = QPixmap::fromImage(image);
            ui->imagen->setPixmap(pixmap.scaled(ui->imagen->width(), ui->imagen->height(), Qt::KeepAspectRatio));
        }else{
            QPixmap pixmap = QPixmap(":/images/logo.png");
            ui->imagen->setPixmap(pixmap.scaled(ui->imagen->width(), ui->imagen->height(), Qt::KeepAspectRatio));
        }

    }else{
        set_to_not_found();
    }
}


void Checador::on_pushButton_clicked()
{
    close();
}


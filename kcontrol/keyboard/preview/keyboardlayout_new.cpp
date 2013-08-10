 
#include "keyboardlayout_new.h"

#include <QDebug>
#include <QString>
#include <QList>

KbKey::KbKey(){
    symbolCount = 0;
    symbols << QString();
}

void KbKey::setKeyName(QString n){
        keyName = n;
}

void KbKey::addSymbol(QString n, int i){
    symbols[i] = n;
    symbolCount++;
    symbols << QString();
}


QString KbKey::getSymbol(int i){
    if(i < symbolCount)
        return symbols[i];
    else
        return QString();
}

void KbKey::display(){
    qDebug()<<keyName<<" : ";
    for(int i=0; i<symbolCount; i++)
        qDebug()<<"\t"<<symbols[i];
}


KbLayout::KbLayout(){
    keyCount = 0;
    includeCount = 0;
    keyList << KbKey();
    include << QString();
}

void KbLayout::setName(QString n){
    name = n;
}

void KbLayout::addInclude(QString n){
    if(!include.contains(n)){
        include[includeCount] = n;
        includeCount++;
        include << QString();
    }
}


void KbLayout :: addKey(){
    keyCount++;
    keyList << KbKey();
}

QString KbLayout :: getInclude(int i){
    if(i < includeCount)
        return include[i];
    else
        return QString();
}

int KbLayout :: findKey(QString n){
    for(int i = 0 ; i < keyCount ; i++){
        if(keyList[i].keyName == n){
            return i;
        }
    }
    return -1;
}

void KbLayout::display(){
    qDebug()<< name <<"\n";
    for(int i = 0; i<includeCount; i++){
        qDebug()<<include[i];
    }
    for(int i = 0 ; i<keyCount; i++ ){
        keyList[i].display();
    }
}

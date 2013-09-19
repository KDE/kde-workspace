
#include "model_to_geometry.h"


ModelGroup :: ModelGroup(){
    modelCount = 0;
    groupModels << QString();
}

void ModelGroup :: addGroupModel(QString model){
    if( !groupModels.contain(model) ){
        groupModels << model;
        modelCount++;
        groupModels << QString;
    }
}

QString ModelGroup :: getGroupModel( int i ){
    if(i < modelCount ){
        return groupModels.at(i);
    }
    else
        return -1;
}


QString ModelGroup :: containsModel(QSting model){
    for(int i = 0 ; i < modelCount; i++){
        if (model == groupModels.at(i)){
            return i;
        }
    }
    else
        return -1;
}

ModelToGeometryTable :: ModelToGeometryTable(){

    table << ModelToGeometry();
    modelGroups << ModelGroup();
    entryCount = 0;
    modelGroupCount = 0;

}

QString ModelToGeometryTable :: getGeometryFile(QString model){

    for (int i = 0; i < entryCount ; i++){

        ModelToGeometry entry = table.at(i);
        QString tempModel = entry.getModel();

        if(tempModel.startsWith('$')){
            for ( int j = 0; j < modelGroupCount; j++){
                ModelGroup temp = modelGroups.at(j);
                if (temp.containsModel(model) > -1){
                    return entry.getGeometryFile();
                }
            }
        }
        else{
            if (tempModel == model){
                return entry.getGeometryFile();
            }
        }
        return defaultGeometryFile;
    }

}


QString ModelToGeometryTable :: getGeometryName(QString model){

    for (int i = 0; i < entryCount ; i++){

        ModelToGeometry entry = table.at(i);
        QString tempModel = entry.getModel();

        if(tempModel.startsWith('$')){
            for ( int j = 0; j < modelGroupCount; j++){
                ModelGroup temp = modelGroups.at(j);
                if (temp.containsModel(model) > -1){
                    return entry.getGeometryName();
                }
            }
        }
        else{
            if (tempModel == model){
                return entry.getGeometryFile();
            }
        }
        return defaultGeometryName;
    }

}



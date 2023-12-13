#pragma once

#include <QString>
#include <QMap>

enum EPathType { Tabulation, BinSearch };

enum EStatus { Running, Stopped };

enum EAction { Suspend, Priority, Affinity, Terminate };

QMap<EPathType, std::string> MPathMap = {
    {Tabulation,
     "/Users/bossofthisgym/build-tabulation-Desktop_arm_darwin_generic_mach_o_64bit-Debug/tabulation"},
    {BinSearch,
     "/Users/bossofthisgym/build-binserch-Desktop_arm_darwin_generic_mach_o_64bit-Debug/binserch"},
    };

QMap<EPathType, std::string> MProcessNameMap = {
    {Tabulation, "tabulation"},
    {BinSearch, "binserch"}
};

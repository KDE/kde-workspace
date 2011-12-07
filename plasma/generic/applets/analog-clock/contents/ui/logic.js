function daysInMonth(month)
{
    if (month==2) {
        if (isLeap(year)) return 29;
        else return 28;
    }
    else if (month<8) {
        if (month%2==1) return 31;
        else return 30;
    }
    else {
        if (month%2==0) return 31;
        else return 30;
    }
}

function isLeap(year)
{
    return ((year%100==0 && year%400==0) || (year%4==0 && year%100!=0));
}

function getYear(date)
{
    return parseInt(Qt.formatDate(date, "yyyy"));
}

function getMonth(date)
{
    return parseInt(Qt.formatDate(date, "M"));
}

function getDate(date)
{
    return parseInt(Qt.formatDate(date, "d"));
}

function getWeekday(date)
{
}


function shouldOfferSuspend(pmSource) {
    return pmSource.data["Sleep States"]["Suspend"];
}

function shouldOfferHibernate(pmSource) {
    return pmSource.data["Sleep States"]["Hibernate"];
}


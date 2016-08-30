window.addEventListener('load', function() {
    if ('serviceWorker' in navigator) {
        navigator.serviceWorker.register('./sw.js').then(function(registration) {
            console.log('ServiceWorker: Registration Succeed. Scope: ', registration.scope);
        }).catch(function(err) {
            console.log('ServiceWorker: Registration Failed.  Message: ', err);
        });
    }
});

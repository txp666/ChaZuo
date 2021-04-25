jQuery("#makeQRCode").bind("click", function() {

    var text = jQuery('#deviceInfo').val();
    jQuery(function() {
        jQuery('#qrcode').empty();
        jQuery('#qrcode').qrcode({
            render: "canvas",
            width: 120,
            height: 120,
            text: text

        })
    })

});
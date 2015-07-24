(function ($) {
    Drupal.behaviors.initModalBynderAddForm = {
        attach: function (context, settings) {
            'use strict';

            var spinner;

            var showAlert = function (message, type) {
                var alert = $('#edit-bynder-search .alert');
                alert.find('span.text').text(message);
                alert.removeClass('alert-warning').removeClass('alert-success').removeClass('alert-error');
                alert.addClass('alert-' + type);
                alert.fadeIn();
                setTimeout(function () {
                    alert.fadeOut(1000);
                }, 2500);
            };

            var showSpinner = function (element) {
                spinner = new Spinner({
                    lines: 13,
                    length: 20,
                    width: 10,
                    radius: 30,
                    corners: 1,
                    rotate: 0,
                    direction: 1,
                    color: '#000',
                    speed: 1,
                    trail: 60,
                    shadow: false,
                    hwaccel: false,
                    className: 'spinner',
                    zIndex: 2e9,
                    top: '50%',
                    left: '50%'
                }).spin(element);
            };

            // Public variables/methods
            var bynder_is_valid = function (type, val) {
                switch (type) {
                    case "uuid":
                        return /^[0-9a-fA-F]{8}-?[0-9a-fA-F]{4}-?[0-9a-fA-F]{4}-?[0-9a-fA-F]{4}-?[0-9a-fA-F]{12}$/.test(val);
                    case "idHash":
                        return /[a-f0-9]{16}/.test(val);
                }
                return true;
            };

            var bynder_validate_uuid = function (str) {
                if (bynder_is_valid("uuid", str)) {
                    return str;
                } else {
                    throw "Invalid UUID: " + str;
                }
            };

            var bynder_validate_id_hash = function (str) {
                if (bynder_is_valid("idHash", str)) {
                    return str;
                } else {
                    throw "Invalid IDHash: " + str;
                }
            };

            $('#edit-bynder-search .normal_facet_list > .facet_title').click(function () {
                var $filters = $(this).siblings();
                $filters.find('.expand i.fa-angle-up')
                    .addClass('fa-angle-down')
                    .removeClass('fa-angle-up');
                $filters.find('.item-list:visible').slideUp(300);

                $(this).find('.expand i').toggleClass('fa-angle-down').toggleClass('fa-angle-up');
                $(this).find(' > .item-list').slideToggle(300);
            });

            $('#edit-bynder-search .selected_facet_list > .facet_title').click(function (e) {
                var link = $(e.currentTarget);
                var filters_input = $('#edit-bynder-search input[name="filters"]');

                var filter_key = link.data('filter-key');
                var current_filters = JSON.parse(filters_input.val() || '{}');
                current_filters.filters = current_filters.filters || [];

                var exists = ($.grep(current_filters.filters, function (e, i) {
                    return e.key == filter_key;
                }).length);
                if (exists) {
                    //Remove current filter from list
                    current_filters.filters = $.grep(current_filters.filters, function (e, i) {
                        return e.key != filter_key;
                    });
                    link.removeClass('active');
                }
                filters_input.val(JSON.stringify(current_filters));
                link.parent().fadeOut(300, function () {
                    $(this).remove();
                });
                $('#bynder-add-form').submit();
            });

            $('#edit-bynder-search .filter-url').click(function (e) {
                e.preventDefault();

                var link = $(e.currentTarget);
                var filter_key = link.data('filter-key');
                var filter_value = link.data('filter-value');

                var filters_input = $('#edit-bynder-search input[name="filters"]');

                var current_filters = JSON.parse(filters_input.val() || '{}');
                current_filters.filters = current_filters.filters || [];

                var exists = ($.grep(current_filters.filters, function (e, i) {
                    return e.key == filter_key;
                }).length);
                if (exists) {
                    //Remove current filter from list
                    current_filters.filters = $.grep(current_filters.filters, function (e, i) {
                        return e.key != filter_key;
                    });
                    link.removeClass('active');
                }

                // Add current filter to list
                current_filters.filters.push({
                    key: filter_key,
                    value: filter_value
                });
                link.addClass('active');

                filters_input.val(JSON.stringify(current_filters));
                $('#bynder-add-form').submit();
            });

            $('#edit-bynder-search .result_list .bynder-image').click(function (e) {
                e.preventDefault();
                e.stopPropagation();
                var $image = $(e.currentTarget);
                $image.addClass('loading');

                showSpinner($image[0]);

                var path = "bynder://"+$image.attr('data-path');
console.log(path);
                var field_name = Drupal.settings.bynder.field_name;
                var field_delta = Drupal.settings.bynder.field_delta;

                var input_url_name =  field_name + '[und][' + field_delta + '][filefield_bynder][url]';

                var input_submit_name =  field_name + '_und_' + field_delta + '_submit';
console.log(input_url_name);
                var input_url = $('input[name="' + input_url_name + '"]');
                var input_submit = $('input[name="' + input_submit_name + '"]');
                input_url.val(path);
                input_submit.mousedown();
                Drupal.CTools.Modal.dismiss();


            });


            $('#bynder-add-form').submit(function () {
                showSpinner($('#edit-bynder-search')[0]);
            });
        }
    };

})(jQuery);

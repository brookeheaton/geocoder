<?php
// $Id$

/**
 * @file
 * Plugin to provide a texas_am geocoder.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Texas A&M Geoservices"),
  'description' => t('Geocodes via Texas A&M Geoservices'),
  'callback' => 'geocoder_texas_am',
  'field_types' => array('addressfield'),
  'field_callback' => 'geocoder_texas_am_field',
  'terms_of_service' => 'https://geoservices.tamu.edu/About/Legal/TermsOfUse.aspx',
);

/**
 * Process Markup
 */
function geocoder_texas_am($address, $options = array()) {
  try {
    geophp_load();
    $geocoder_settings = variable_get('geocoder_settings', array());

    if (!empty($geocoder_settings['geocoder_apikey_texas_am'])) {
      $consumer_key = $geocoder_settings['geocoder_apikey_texas_am'];
    }
    else {
      throw new Exception('You must specify a key.');
    }
    if (isset($options['address']) && is_array($options['address'])) {
      $query = array(
        'state' => (isset($options['address']['administrative_area']) ? $options['address']['administrative_area'] : ''),
        'city' => (isset($options['address']['locality']) ? $options['address']['locality'] : ''),
        'zip' => (isset($options['address']['postal_code']) ? $options['address']['postal_code'] : ''),
        'streetAddress' => (isset($options['address']['thoroughfare']) ? $options['address']['thoroughfare'] : ''),
        'apiKey' => $consumer_key,
      );
    }
    else {
      $query = array(
        'streetAddress' => $address,
        'apiKey' => $consumer_key,
      );
    }
    $url = url('http://geoservices.tamu.edu/Services/Geocode/WebService/GeocoderWebServiceHttpNonParsed_V04_01.aspx?version=4.01&format=json', array('query' => $query));
    $result = drupal_http_request($url);

    if (isset($result->error)) {
      $args = array(
        '@code' => $result->code,
        '@error' => $result->error,
      );
      $msg = t('HTTP request to Texas A&M Geoservices API failed.\nCode: @code\nError: @error', $args);
      throw new Exception($msg);
    }

    $data = json_decode($result->data);

    if ($data->QueryStatusCodeValue != 200) {
      $msg = t('Texas A&M Geoservices API returned bad status.\nStatus: @status\n@description', array(
        '@status' => $data->QueryStatusCodeValue,
        '@description' => $data->Exception,
      ));
      throw new Exception($msg);
    }
  } catch
  (Exception $e) {
    watchdog_exception('geocoder', $e);
    return FALSE;
  }
  geophp_load();
  return _geocoder_texas_am_geometry($data);
}

      function geocoder_texas_am_field($field, $field_item) {
        if ($field['type'] == 'addressfield' && module_exists('addressfield') && !addressfield_field_is_empty($field_item, $field)) {
          $address = $field_item;
          return geocoder_texas_am($address, array('address' => $address));
        }
        if ($field['type'] == 'location') {
          $address = geocoder_widget_parse_locationfield($field_item);
          return geocoder_texas_am($address, array('address' => $address));
        }
      }

      function _geocoder_texas_am_geometry(&$data) {
        if (!isset($data->OutputGeocodes[0]->OutputGeocode->Longitude, $data->OutputGeocodes[0]->OutputGeocode->Latitude)) {
          return NULL;
        }
        else {
          return new Point($data->OutputGeocodes[0]->OutputGeocode->Longitude, $data->OutputGeocodes[0]->OutputGeocode->Latitude);
        }
      }




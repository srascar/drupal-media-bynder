# Bynder Module

This module provide a direct link to the Bynder API trough a modal to explore and select the medias.
The module is designed to be used with an **image field ** or a ** file field **.

It creates a new file sources entry called **File attach from Bynder** in order to keep other contribution methods on the same field and the additional informations.
Actually when linking a picture we are not getting the metadatas of the media. We just get the external thumbnail to display it on the node form.
The modal keeps the search and filters functionnalities from the original Bynder Media Module.

The module creates a new stream wrapper called "bynder" and store me media URLs like **bynder://[mediaId]** in te *managed files* database. The module does not provide any image presets. The original *uri_parse_style_url* function had been kept but is not used instead we do have a *bynder_embed_url* function returning the public couldfront URL of the selected profile.

### Patch note
Actually we had to patch the Drupal core in the image module for the *hook_field_validate* and the undetermined width and height of the pictures.
https://www.drupal.org/files/issues/image-validate-1330952-63-d7.patch

### Dependencies
* oauth_common To manage the REST API
* Ctools to manage the modal and ajax
* That's it! :)

### Next steps
The module provides only the needed functionnalities for the current project. This is just a starter for a global Bynder module, no further developments are actually planned on the module because the current project does not required anything more.
/*
  TODO:
   - native app:
       - store assets paths, simple, no memory cost BUT risk of misses if user moves assets location
       - OR make a dedicated asset directory, auto copy user data to this directory, allows user to manage assets
outside the app
   - web app:
       - use db based asset persistant storage BUT limited size, issue for high res images/videos
       - AND/OR `FileSystemFileHandle` BUT this only works for chrome + needs additional permissions + risk of misses if
user moves assets location
       - OR serialize ressources skipping the need of asset management and make the user actively save/reload projects including these ressources


   CURRENTLY:
    - No persistent data, load directly ressources when user import them but do not allow resuming a project
 */

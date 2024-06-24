# Inline editing

Inline editing allows you to edit the metadata of tracks directly in the
playlist view. In order to use inline editing, linked metadata fields have to be
assigned to columns in Preferences.

Once configured, inline editing can be activated by clicking on a field of a
selected track in the playlist view, or by pressing the F2 key while the
playlist view is focused.

If multiple tracks are selected, the F2 key can be used to edit the same field
across multiple tracks. (This allows setting the field to the same value for all
tracks being edited only.)

The following keys can be used to navigate while inline editing is active:

| Key        | Action                                                                                      |
| ---------- | ------------------------------------------------------------------------------------------- |
| Esc        | Cancel changes and exit inline editing mode                                                 |
| Enter      | Save changes and exit inline editing mode                                                   |
| Tab        | Save changes and advance to the next field (or track when on the last editable column)      |
| Shift+Tab  | Save changes and advance to the previous field (or track when on the first editable column) |
| Ctrl+Enter | Insert a newline                                                                            |
| Up or Down | Navigate within a multiline field                                                           |

Clicking outside the edit box will save changes and exit inline editing.

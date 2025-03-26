from django.urls import path
from .views import export_reseau_json, export_reseau_xml

urlpatterns = [
    path('export-reseau/', export_reseau_json, name='export_reseau_json'),    
    path('export-reseau-xml/', export_reseau_xml, name='export_reseau_xml'),
]

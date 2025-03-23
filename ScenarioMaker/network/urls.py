from django.urls import path
from .views import export_reseau_json

urlpatterns = [
    path('export-reseau/', export_reseau_json, name='export_reseau_json'),
]

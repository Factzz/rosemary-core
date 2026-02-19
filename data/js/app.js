const state = { plants: [] };

async function loadData() {
    try {
        const res = await fetch('/api/data');
        const data = await res.json();
        state.plants = data.plants || [];
        
        // Update Stats
        document.getElementById('vpd-val').innerText = data.env.vpd.toFixed(2);
        document.getElementById('temp-val').innerText = data.env.temp.toFixed(1);
        document.getElementById('hum-val').innerText = data.env.hum.toFixed(0);
        
        renderList();
    } catch(e) { console.log("Connection lost..."); }
    setTimeout(loadData, 2000);
}

function renderList() {
    const list = document.getElementById('plant-list');
    if(!list) return;
    
    list.innerHTML = state.plants.map(p => {
        let statusTag = "";
        if(p.is_watering) statusTag = `<span class="tag water">PUMPING</span>`;
        else if(p.error) statusTag = `<span class="tag err">ERROR</span>`;
        else if(p.moisture < p.threshold) statusTag = `<span class="tag dry">DRY</span>`;
        
        let sensorType = (p.sensor_mode && p.sensor_mode.includes("Radar")) ? "RADAR" : "ANALOG";

        return `
        <div class="plant-row">
            <div class="p-info">
                <div class="p-name">${p.name} ${statusTag}</div>
                <div class="p-meta">CH:${p.originalIndex} | ${sensorType} | TH:${p.threshold}%</div>
                <div class="actions">
                    <button onclick="app.water(${p.originalIndex})">WATER</button>
                    <button class="btn-del" onclick="app.del(${p.id})">RM</button>
                </div>
            </div>
            <div class="p-val">${p.moisture}%</div>
        </div>`;
    }).join('');
}

window.app = {
    openAdd: () => document.getElementById('modal-add').style.display = 'flex',
    openWifi: () => document.getElementById('modal-wifi').style.display = 'flex',
    closeModals: () => document.querySelectorAll('.modal-overlay').forEach(e => e.style.display = 'none'),
    
    addPlant: async () => {
        const name = document.getElementById('new-name').value;
        if(name) {
            // Auto-detect sensor logic is handled by firmware, just send basic config
            await fetch('/api/add-plant', { 
                method:'POST', 
                headers:{'Content-Type':'application/json'},
                body: JSON.stringify({name: name, type: 'general', threshold: 40}) 
            });
            app.closeModals();
            loadData();
        }
    },
    
    water: (idx) => fetch('/api/water', { method:'POST', body:JSON.stringify({index:idx}) }),
    del: (id) => { if(confirm('Confirm Delete?')) fetch('/api/delete-plant', { method:'POST', body:JSON.stringify({id:id}) }); },
    
    saveWifi: () => {
        const s = document.getElementById('wifi-ssid').value;
        const p = document.getElementById('wifi-pass').value;
        fetch('/api/save-wifi', { method:'POST', body:JSON.stringify({ssid:s, password:p}) });
        alert('Rebooting...');
    },
    reboot: () => { if(confirm('Reboot?')) fetch('/api/reboot', {method:'POST'}); }
};

loadData();
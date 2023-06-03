function sh = SH1_CURVE(t,x4)
    % Function calculates time delay for HU1
    if t <= 0.0
        sh=0.0;
    elseif t < x4
        sh=(t/x4)^(2.5);
    elseif t >= x4
        sh=1.0;
    end
end